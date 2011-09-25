/*
 * Дополняет строку cmd так, чтобы весь вывод шел в /dev/null
 */

void execInBkg(char *cmd){
    strcat(cmd, " > /dev/null 2> /dev/null &");
    system(cmd);
}
