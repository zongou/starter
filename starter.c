// read k-v from config
// setup env
// setup path
// start programs

#ifdef __WINNT__
#include <windows.h>
#include <strsafe.h>
#endif

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUF_LEN 1024
#define MAX_KEY_LEN 64
#define MAX_VAL_LEN 256
#define BUFSIZE 4096

int readArgs(int argc, char *argv[]) { return 0; }

int setup_env(char *v, int line_number) {
  // arg example: env=relative,JAVA_HOME,./jdk/jdk11/
  char delimiter[] = ",";

  char *ptr = strtok(v, delimiter);

  const int MAX_SIZE = 3;
  char *data[MAX_SIZE];

  for (int line_numberer = 0; ptr != NULL; line_numberer++) {

    if (line_numberer < MAX_SIZE) {
      data[line_numberer] = ptr;
      ptr = strtok(NULL, delimiter);

    } else {
      printf("[Error]config file line %d error\n", line_number);
      return 1;
    }
  }

#ifdef __WINNT__
  char *env_value;
  if(strcasecmp(data[0], "static")==0){
    env_value=data[2];
  }else if (strcasecmp(data[0], "relative")==0) {
    char canonicalPath[MAX_PATH];
    GetFullPathName(data[2], MAX_PATH, canonicalPath, 0);
    env_value=canonicalPath;
  }
  else {
    printf("[Error]config file line %d error", line_number);
    return 1;
  }

  if (!SetEnvironmentVariable(data[1], env_value)) {
    printf("SetEnvironmentVariable failed (%lu)\n", GetLastError());
    return 1;
  }
#else
#endif
  return 0;
}

int setup_path(char *v, int line_number) {
#ifdef __WINNT__
  DWORD dwRet, dwErr;
  BOOL fExist, fSuccess;
  LPTSTR pszOldVal = malloc(BUFSIZE * sizeof(TCHAR));
  LPSTR VARNAME = TEXT("PATH");
  dwRet = GetEnvironmentVariable(VARNAME, pszOldVal, BUFSIZE);
  if (dwRet == 0) {
    dwErr = GetLastError();
    if (ERROR_ENVVAR_NOT_FOUND == dwErr) {
      printf("Environment variable %s does not exist.\n", VARNAME);
      fExist = 1;
    }
  } else if (BUFSIZE < dwRet) {
    pszOldVal = (LPTSTR)realloc(pszOldVal, dwRet * sizeof(TCHAR));
    if (NULL == pszOldVal) {
      printf("Out of memory\n");
      return 1;
    }
    dwRet = GetEnvironmentVariable(VARNAME, pszOldVal, dwRet);
    if (!dwRet) {
      printf("GetEnvironmentVariable failed (%lu)\n", GetLastError());
      return 1;
    } else
      fExist = TRUE;
  } else
    fExist = TRUE;

  HRESULT hresult;
  char canonicalPath[MAX_PATH];
  GetFullPathName(v, MAX_PATH, canonicalPath, 0);
  hresult = StringCchCat(pszOldVal, BUFSIZE, canonicalPath);
  if (hresult != S_OK) {
    printf("StringCbCopy Failed: %s", pszOldVal);
    return 1;
  }
  hresult = StringCchCat(pszOldVal, BUFSIZE, ";");
  if (hresult != S_OK) {
    printf("StringCbCopy Failed: %s", pszOldVal);
    return 1;
  }

  if (!SetEnvironmentVariable(VARNAME, pszOldVal)) {
    printf("SetEnvironmentVariable failed (%lu)\n", GetLastError());
    return 1;
  }
  free(pszOldVal);
#elifdef __linux__
  char *path = getenv("PATH");
  path = strcat(path, v);
  path = strcat(path, ":");
#else
  path = strcat(path, ":");
#endif
  // putenv(path);
  return 0;
}

int start_program(char *v) {
  printf("run program here\n");
#ifdef __WINNT__
  printf("start program on windows");
  DWORD dwRet, dwErr;
  LPTSTR pszOldVal;

  STARTUPINFO si = {sizeof(si)};
  PROCESS_INFORMATION pi;
  BOOL fSuccess;
  DWORD dwFlags = CREATE_NEW_CONSOLE;
  LPSTR szCmd = v;

#ifdef UNICODE
  dwFlags = CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT;
#endif

  fSuccess = CreateProcess(NULL,    // No module name (use command line)
                           szCmd,   // Command line
                           NULL,    // Process handle not inheritable
                           NULL,    // Thread handle not inheritable
                           FALSE,   // Set handle inheritance to FALSE
                           dwFlags, // No creation flags
                           NULL,    // Use parent's environment block
                           NULL,    // Use parent's starting directory
                           &si,     // Pointer to STARTUPINFO structure
                           &pi);    // Pointer to PROCESS_INFORMATION structure
  if (!fSuccess) {
    printf("CreateProcess failed (%lu)\n", GetLastError());
  }
  WaitForSingleObject(pi.hProcess, INFINITE);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
#elifndef __linux__
  printf("start program on linux");
#endif
  return 0;
}

int Trim(char s[]) {
  int n;
  for (n = strlen(s) - 1; n >= 0; n--) {
    if (s[n] != ' ' && s[n] != '\t' && s[n] != '\n')
      break;
    s[n + 1] = '\0';
  }
  return n;
}

int loadConfigDemo(const char *config_path) {
  FILE *file = fopen(config_path, "r");
  if (file == NULL) {
    printf("[Error]open %s failed.\n", config_path);
    return -1;
  }
  int line_number = 0;

  char buf[MAX_BUF_LEN];
  int text_comment = 0;
  while (fgets(buf, MAX_BUF_LEN, file) != NULL) {
    Trim(buf);
    // to skip text comment with flags /* ... */
    if (buf[0] != '#' && (buf[0] != '/' || buf[1] != '/')) {
      if (strstr(buf, "/*") != NULL) {
        text_comment = 1;
        line_number++;
        continue;
      } else if (strstr(buf, "*/") != NULL) {
        text_comment = 0;
        line_number++;
        continue;
      }
    }
    if (text_comment == 1) {
      line_number++;
      continue;
    }

    int buf_len = strlen(buf);
    // ignore and skip the line with first chracter '#', '=' or '/'
    if (buf_len <= 1 || buf[0] == '#' || buf[0] == '=' || buf[0] == '/' ||
        buf[0] == ';') {
      line_number++;
      continue;
    }
    buf[buf_len - 1] = '\0';

    char _paramk[MAX_KEY_LEN] = {0}, _paramv[MAX_VAL_LEN] = {0};
    int _kv = 0, _klen = 0, _vlen = 0;
    int i = 0;
    for (i = 0; i < buf_len; ++i) {
      // if (buf[i] == ' ')
      //   continue;
      // scan param key name
      if (_kv == 0 && buf[i] != '=') {
        if (_klen >= MAX_KEY_LEN){
          line_number++;
          break;
        }
        _paramk[_klen++] = buf[i];
        continue;
      } else if (buf[i] == '=') {
        _kv = 1;
        line_number++;
        continue;
      }
      // scan param key value
      if (_vlen >= MAX_VAL_LEN || buf[i] == '#'){
        line_number++;
        break;
      }
      _paramv[_vlen++] = buf[i];
    }
    if (strcmp(_paramk, "") == 0 || strcmp(_paramv, "") == 0){
      line_number++;
      continue;
    }
    printf("%s=%s\n", _paramk, _paramv);

    if (strcasecmp(_paramk, "PATH") == 0) {
      if(setup_path(_paramv, line_number)) return 1;
    } else if (strcasecmp(_paramk, "ENV") == 0) {
      if(setup_env(_paramv, line_number)) return 1;
    } else if (strcasecmp(_paramk, "PROGRAM") == 0) {
      if(start_program(_paramv)) return 1;
    }
  }
  return 0;
}

int hide_console() {
#ifdef __WINNT__
  HWND hwnd;
  hwnd = FindWindow("ConsoleWindowClass", NULL);
  if (hwnd) {
    ShowWindow(hwnd, SW_HIDE); //设置指定窗口的显示状态
  }
  // MessageBoxW(NULL,L"控制台已隐藏",L"提示",MB_OK);
#endif
  return 0;
}

int main(int argc, char *argv[]) {
  hide_console();
  if(loadConfigDemo("./starter.conf")){
    return 1;
  }
  return 0;
}
