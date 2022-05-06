 #include <Strsafe.h>
int main(int argc, char *argv[])
{
  char MyString[128];
  HRESULT Res;
  Res=StringCbCopy(MyString, sizeof(MyString), "Program 1. Name is ");
  if (Res != S_OK) {
    printf("StringCbCopy Failed: %s", MyString);
    return 1;
  }
  Res=StringCbCat(MyString,sizeof(MyString),argv[0]);
  if (Res != S_OK) {
    printf("StringCbCat Failed: %s", MyString);
    return 1;
  }
  printf("%s", MyString);
  return 0;
}