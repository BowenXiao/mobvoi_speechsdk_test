#include<iostream>
#include<cstring>
using namespace std;
int main()
{
  /* code */
  char sentence[]="This is a sentence with 7 tokens";
  cout << "The string to be tokenized is :\n" << sentence << "\n\nThe tokens are:\n\n";
  char *tokenPtr=strtok(sentence," ");
  while(tokenPtr != NULL) {
    cout << tokenPtr << '\n';
    tokenPtr=strtok(NULL," ");
  }
  return 0;
}