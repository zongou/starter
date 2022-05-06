// first, read k-v from ini
// second, setup env
// third, runs commands
// fourth, start programs

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __WINNT__
#include <windows.h>
#include <strsafe.h>
#endif

#define MAX_BUF_LEN 1024
#define MAX_KEY_LEN 64
#define MAX_VAL_LEN 256
#define BUFSIZE 4096

int setup_env();
int run_commands();
int start_program();

int readArgs(int argc, char *argv[]) { return 0; }

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

  char buf[MAX_BUF_LEN];
  int text_comment = 0;
  while (fgets(buf, MAX_BUF_LEN, file) != NULL) {
    Trim(buf);
    // to skip text comment with flags /* ... */
    if (buf[0] != '#' && (buf[0] != '/' || buf[1] != '/')) {
      if (strstr(buf, "/*") != NULL) {
        text_comment = 1;
        continue;
      } else if (strstr(buf, "*/") != NULL) {
        text_comment = 0;
        continue;
      }
    }
    if (text_comment == 1) {
      continue;
    }

    int buf_len = strlen(buf);
    // ignore and skip the line with first chracter '#', '=' or '/'
    if (buf_len <= 1 || buf[0] == '#' || buf[0] == '=' || buf[0] == '/') {
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
        if (_klen >= MAX_KEY_LEN)
          break;
        _paramk[_klen++] = buf[i];
        continue;
      } else if (buf[i] == '=') {
        _kv = 1;
        continue;
      }
      // scan param key value
      if (_vlen >= MAX_VAL_LEN || buf[i] == '#')
        break;
      _paramv[_vlen++] = buf[i];
    }
    if (strcmp(_paramk, "") == 0 || strcmp(_paramv, "") == 0)
      continue;
    // printf("%s=%s\n", _paramk, _paramv);

    if (strcasecmp(_paramk, "PATH") == 0) {
      setup_env(_paramv);
    } else if (strcasecmp(_paramk, "COMMAND") == 0) {
      run_commands(_paramv);
    } else if (strcasecmp(_paramk, "PROGRAM") == 0) {
      start_program(_paramv);
    }
  }
  return 0;
}

int setup_env(char *v) {

#ifdef __linux__
  char *path = getenv("PATH");
  path = strcat(path, v);
  path = strcat(path, ":");
#elifdef __WINNT__
  DWORD dwRet, dwErr;
  BOOL fExist, fSuccess;
  LPTSTR pszOldVal = malloc(BUFSIZE * sizeof(TCHAR));
  LPSTR VARNAME = TEXT("PATH");
  dwRet = GetEnvironmentVariable(VARNAME, pszOldVal, BUFSIZE);
  if (dwRet == 0) {
    dwErr = GetLastError();
    if (ERROR_ENVVAR_NOT_FOUND == dwErr) {
      printf("Environment variable does not exist.\n");
      fExist = FALSE;
    }
  } else if (BUFSIZE < dwRet) {
    pszOldVal = (LPTSTR)realloc(pszOldVal, dwRet * sizeof(TCHAR));
    if (NULL == pszOldVal) {
      printf("Out of memory\n");
      return FALSE;
    }
    dwRet = GetEnvironmentVariable(VARNAME, pszOldVal, dwRet);
    if (!dwRet) {
      printf("GetEnvironmentVariable failed (%d)\n", GetLastError());
      return FALSE;
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
    printf("SetEnvironmentVariable failed (%d)\n", GetLastError());
    return FALSE;
  }
  free(pszOldVal);
#else
  path = strcat(path, ":");
#endif
  // putenv(path);
  return 0;
}

int run_commands(char *v) {
  printf("run commands here\n");
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
    printf("CreateProcess failed (%d)\n", GetLastError());
  }
  WaitForSingleObject(pi.hProcess, INFINITE);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  return fSuccess;
#elifndef __linux__
  printf("start program on linux");
#endif
  return 0;
}

int main(int argc, char *argv[]) { loadConfigDemo("./config.ini"); }
