#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>


#define COLOR_GREEN "%c[1;32m"
#define COLOR_YELLOW "%c[1;33m"
#define COLOR_WHITE "%c[1;37m"
#define NULL_CHAR '\0'


void get_input();

int words_length = 0;
int current_line = 0;

char current_word[6];
char user_input[6];
char guess_list[30];

bool initialized = false;
bool game_over = false;


int contains_character(char c)
{

    for(int i = 0; i < 5; i++){
        if (current_word[i] == c)
            return(1);
    }

    return(0);

}


void print_char(char c, char* color)
{
    printf(color, 27);

    if(c)
        printf("%c ", c);
}


void get_word()
{

    FILE* ptr;
    char ch;
    ptr = fopen("words.txt", "r");

    if(!ptr){
        printf("Unable to find or open words.txt \n");
        return;
    }

    // Get length of word list
    if(!initialized){

        srand(time(NULL));
        
        ptr = fopen("words.txt", "r");
        ch = fgetc(ptr);

        int len = 0;
        while (ch != EOF){
            len++;
            ch = fgetc(ptr);
        }

        words_length = len;
        initialized = true;
        
    }

    // Move ptr back to the beginning
    fseek(ptr, 0, SEEK_SET);

    int r = rand() % words_length;
    int i = 0;
    int j = 0;
    int stage = 0;

    // Get a random word from the list
    while(ch != EOF){
        ch = fgetc(ptr);

        // Search for index r
        if(i == r){
            stage = 1;
        }

        // Find the next newline character
        if(stage == 1 && ch == '\n'){
            stage = 2;
        }

        // Capture the next five characters after
        if(stage == 2 && ch != '\n'){
            current_word[j] = (char)toupper(ch);
            j++;
            if(j == 5){
                stage = 3;
                break;
            }  
        }

        i++;
    }

    if(stage != 3)
        get_word();

    fclose(ptr);

}


void clear()
{

    #if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        system("clear");
    #endif

    #if defined(_WIN32) || defined(_WIN64)
        system("cls");
    #endif

}


void string_toupper(char* str)
{

    int i = 0;
    while(str[i]){
        str[i] = toupper(str[i]);
        i++;
    }

}


bool is_input_valid(char* str)
{

    for(int i = 0; i < 5; i++){
        if(!str[i])
            return(false);
    }

    return(!str[5]);

}


void render()
{

    clear();
    int curr = 0;
    for (int i = 0; i < 30; i++) {
        if(!guess_list[i])
            printf("_ ");
        else{
            if(guess_list[i] == current_word[curr]){
                print_char(guess_list[i], COLOR_GREEN);
                print_char(NULL_CHAR, COLOR_WHITE);
            }
            else{
                if(contains_character(guess_list[i])){
                    print_char(guess_list[i], COLOR_YELLOW);
                    print_char(NULL_CHAR, COLOR_WHITE);
                }else
                    print_char(guess_list[i], COLOR_WHITE);
            }
        }

        if((i + 1) % 5 == 0)
            printf("\n");

        curr++;
        if(curr >= 5)
            curr = 0;
    }

}


void check_victory()
{

    bool guessed_right = strcmp(user_input, current_word) == 0;
    render();

    if(current_line > 5 && !guessed_right){
        printf("\nYou lose, the word was: %s \n\n", current_word);
        printf("PLAY | EXIT\n");
        get_input();
    }

    if(guessed_right){
        printf("\nYou win!\n\n");
        printf("PLAY | EXIT\n");
        get_input();
    }else{
        get_input();
        check_victory();
    }

}


void clear_guess_list()
{

    for (int i = 0; i < 30; i++) {
        guess_list[i] = NULL_CHAR;
    }

}


int start_game()
{

    // Current attemps
    current_line = 0;

    clear();
    clear_guess_list();

    get_word();
    check_victory();
    
    return(0);

}


void get_input()
{

    char str[7];

    if(fgets(str, sizeof(str), stdin))
        str[strcspn(str, "\n")] = '\0';

    if(strlen(str) > 5) get_input();


    string_toupper(str);
    if(strcmp(str, "PLAY") == 0) start_game();
    if(strcmp(str, "EXIT") == 0) exit(0);

    if(is_input_valid(str)){
        for(int i = 0; i < 5; i++){
            user_input[i] = str[i];
            guess_list[(5 * current_line) + i] = str[i];
        }

        current_line++;
    }

}


int main()
{

    while(true){
        printf("Teordle - Wordle, but in the terminal\n");
        printf("---------------------------------------\n");
        printf("PLAY | EXIT\n");
        get_input();
        clear();
    }

    return(0);
}
