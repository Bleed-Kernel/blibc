int atoi(const char *s){
    int signage = 0;
    int value   = 0;

    while(*s == ' ' || *s == '\t' || *s == '\n' ||
          *s == '\r' || *s == '\f' || *s == '\v')
        s++;
    
    if (*s == '-'){
        signage = 1;
        s++;
    }else if (*s == '+'){
        s++;
    }

    while (*s >= '0' && *s <= '9'){
        value = value * 10 + (*s - '0');
        s++;
    }

    return value * signage;
}