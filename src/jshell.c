#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fields.h"
#include "jval.h"
#include "dllist.h"
#include "jrb.h"

int main(int argc, char const *argv[])
{
    
    IS is;

    is = new_inputstruct(NULL);

    if(is == NULL) {
        fprintf(stderr, "file cannot be opened");
        return 1;
    }


    while(get_line(is) >= 0){
        if(is->NF > 0 && strcmp(is->fields[0], "#")){
            printf("line: ",is->text1);
        }
    }
    return 0;
}
