SKIP: FAILED


static uint var_1 = 0u;
void main_1() {
  {
    while(true) {
      var_1 = 1u;
      if (false) {
        break;
      }
      var_1 = 3u;
      {
        var_1 = 2u;
      }
      continue;
    }
  }
}

void main() {
  main_1();
}

FXC validation failure:
c:\src\dawn\Shader@0x000002259BAB0390(5,11-14): error X3696: infinite loop detected - loop never exits

