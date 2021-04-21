/************************************************************************/
/***           2. projekt - Práce s datovými strukturami              ***/
/***                                                                  ***/
/***                    Tomáš Homola (xhomol28)                       ***/
/***                                                                  ***/
/*** 06.12.2020                        Ut 8:00-9:50, as. Klobucnikova ***/
/************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

typedef struct{
    char *cell; 
} cell_t;

typedef struct{
    unsigned int columns; 
    cell_t **cells; 
} row_t;

typedef struct {
    unsigned int row_count;
    row_t **rows;
} tabulka_t;

typedef struct{
    int rowX1;
    int colY1;
    int rowX2;
    int colY2;
} cells_selection; 

tabulka_t *init_table ();
void free_function(tabulka_t *table_ptr);
int table_load(tabulka_t *table_ptr, int argc, char *argv[], char *delim);
int del_cmp(char *delim, char c);
int quotation_mark_remove(cell_t *cell);
int table_magnificante(tabulka_t *table_ptr, cells_selection *range);
row_t *add_new_line(tabulka_t *table_ptr);
cell_t *add_cell_to_row(row_t *row_ptr);
int print_sheet(tabulka_t *table_ptr, char *delim);
int last_row(FILE *fp);
int table_alignment(tabulka_t *table_ptr);
int process_CMD_SEQ(char *command, cells_selection *range, tabulka_t *table_ptr);
int CUT_CMD_SEQ(int argc, char *argv[], tabulka_t *table_ptr);
int selection_procces(char *command, cells_selection *range, tabulka_t *table_ptr);
int char_count(char *command, char c);
int find_min(tabulka_t *table_ptr, cells_selection *range);
int find_max(tabulka_t *table_ptr, cells_selection *range);
int find_STR(tabulka_t *table_ptr, cells_selection *range, char *command);
float str_to_num(cell_t *cell, int *valid);
int cell_edit(char *command, cells_selection *range, tabulka_t *table_ptr);
int set_str(tabulka_t *table_ptr, cells_selection *range, char *command);
int cell_clear(tabulka_t *table_ptr, cells_selection *range);
int swap(tabulka_t *table_ptr, cells_selection *range, char *command);
int sum(tabulka_t *table_ptr, cells_selection *range, char *command);
int avg(tabulka_t *table_ptr, cells_selection *range, char *command);
int count(tabulka_t *table_ptr, cells_selection *range, char *command);
int len(tabulka_t *table_ptr, cells_selection *range, char *command);
int struct_edit(char *command, cells_selection *range, tabulka_t *table_ptr);
int irow_arow(char *command, int rowX1, int rowX2, tabulka_t *table_ptr);
int icol_acol(char *command, int colY1, int colY2, tabulka_t *table_ptr);
int insert_to_file(tabulka_t *table_ptr, char delim, char *filename);

int main(int argc, char *argv[])
{
    tabulka_t *table_ptr = init_table();
    char *filename = argv[argc-1];
    //kontrola vstupnych prikazov
    if(argc < 3){
        fprintf(stderr,"Neplatne argmunety\n");
        return -1;
    }
    char *delim = argv[0];
    
    if(strcmp(argv[1],"-d") != 0)
        strcpy(delim, " ");  //0. arg nepouzivam, tak del bude medzera
    else
        delim = argv[2];
    
 
    int res_table_load = table_load(table_ptr,argc,argv,delim);
    if(res_table_load == -1){
        fprintf(stderr,"Nepodarilo sa otvorit tabulku\n");
        return -1;
    }

    table_alignment(table_ptr);
    CUT_CMD_SEQ(argc, argv, table_ptr);
    print_sheet(table_ptr, delim);
    insert_to_file(table_ptr, delim[0], filename);
    free_function(table_ptr);
    return 0;
}

//pointer na zaciatok tabulky
tabulka_t *init_table(){
    tabulka_t *table_ptr = (tabulka_t *) malloc(sizeof(tabulka_t));    
    if(table_ptr == NULL){
        fprintf(stderr, "ERROR");   
        return NULL;
    } 
    row_t **rows = (row_t **) malloc(sizeof(row_t*));
    if(rows == NULL){
        fprintf(stderr, "ERROR");   
        return NULL;
    } 
    table_ptr->rows = rows;
    table_ptr->row_count = 0;
    
    return table_ptr;
}
//skontroluje posledny riadok
int last_row(FILE *fp){
    int znak = fgetc(fp);
    if (znak == EOF){
        ungetc(znak, fp);
        return 1;
    }
    ungetc(znak, fp);
    return 0;
}

//nacitanie tabulky
int table_load(tabulka_t *table_ptr, int argc, char *argv[], char *delim){
    char *filename = argv[argc-1];  //-1 kvoli indexacii
    FILE * fp;
    fp = fopen(filename,"r");

    if(fp == NULL)
        return -1;

    char c;
    int cell_count = 0;

    row_t *line = add_new_line(table_ptr);
    cell_t *cell = add_cell_to_row(line);//obsah bunky
    char *new_cell;         //na realokovanie 
    int x = 100;             //prvotna velkost bunky

    int char_count = 0;     //index aktualneho znaku

    while((c = fgetc(fp)) != EOF && last_row(fp) == 0){        
        //ked je bunka vacsia ako som zadal, tak menim pamat
        if(x == char_count){
            x *= 2;    //Ing. Smrcka PhD povedal
            new_cell = (char *)realloc(cell->cell, sizeof(char) * x + 1);
            if(new_cell == NULL){
                fprintf(stderr,"ERROR");
                break;
            }
            for (int i = x/2; i < x+1; i++)  //cyklus na inicializaciu pola
                new_cell[i] = '\0';
            cell->cell = new_cell;
        }  

        //pridanie novej prazdnej bunky
        if(del_cmp(delim, c) == 1){
            cell->cell[char_count] = '\0';
            cell_count++;
            char_count = 0;
            quotation_mark_remove(cell);
            cell = add_cell_to_row(line);
            x = 100;
            continue;
        }
        //detekcia noveho riadku
        if(c == '\n'){
            cell->cell[char_count] = '\0';
            cell_count = 0; //po kazdom koncu riadka sa pocitanie buniek vynuluje 
            line = add_new_line(table_ptr);
            quotation_mark_remove(cell);
            cell = add_cell_to_row(line);
            char_count = 0;
            continue;
        } 
        cell->cell[char_count] = c;
        char_count++;       
    }
    quotation_mark_remove(cell);
    fclose(fp);
    return 0;
}

//vrati 1, ak sa nacitany znak = delimitru, inak vrati -1
int del_cmp(char *delim, char c){
    char *pos;
    pos = strchr(delim, c);
    if(pos != NULL)
        return 1;

    return -1;           
}
//odstrani " " ak su na zaciatku a na konci bunky
int quotation_mark_remove(cell_t *cell){
    if(strncmp(cell->cell, "\"", 1) == 0 && cell->cell[strlen(cell->cell)-1] == '\"'){
        for(int i = 0; i < (int)strlen(cell->cell); i++){
            cell->cell[i] = cell->cell[i+1];
        }
        cell->cell[strlen(cell->cell)-1] = '\0';
    }
    return 0;
}

//prida (nacita) novy riadok
row_t *add_new_line(tabulka_t *table_ptr){
    row_t **resized_rows = (row_t**)realloc(table_ptr->rows, sizeof(row_t*) * (table_ptr->row_count +1));
    if(resized_rows == NULL){
        fprintf(stderr,"ERROR\n");
        return NULL;
    }
    table_ptr->rows = resized_rows;
    row_t *line = (row_t*) malloc(sizeof(row_t));
    if(line == NULL){
        fprintf(stderr,"ERROR\n");
        return NULL;
    }
    table_ptr->rows[table_ptr->row_count] = line;
    table_ptr->row_count++;
    line->columns = 0;
    cell_t **rows = (cell_t**)malloc(sizeof(cell_t*));
    if(rows == NULL){
        fprintf(stderr,"ERROR\n");
        return NULL;
    }
    line->cells = rows;

    return line;
}

//prida novu bunku do riadku
cell_t *add_cell_to_row(row_t *row_ptr){
    cell_t** cells = (cell_t**)realloc(row_ptr->cells, sizeof(cell_t*) * (row_ptr->columns+1));  
    if(cells == NULL){
        fprintf(stderr,"ERROR\n");
        return NULL;
    }
    row_ptr->cells = cells; 

    cell_t* cell = (cell_t*)malloc(sizeof(cell_t));
    if(cell == NULL){
        fprintf(stderr,"ERROR\n");
        return NULL;
    }
    row_ptr->cells[row_ptr->columns] = cell;
    row_ptr->columns++;

    char *cell_content;         //obsah bunky
    cell_content = (char *)malloc(sizeof(char) * 100 + 1); //prvotna velkost bunky {100}
    if(cell_content == NULL){
        fprintf(stderr,"ERROR\n");
        return NULL;
    }
    for (int i = 0; i < 100; i++){  //cyklus na inicializaciu pola
        cell_content[i] = '\0';
    }
    cell->cell = cell_content; 
    return cell;
}

//print tabulky
int print_sheet(tabulka_t *table_ptr, char *delim){
    for(unsigned int i = 0; i < table_ptr->row_count; i++){
        for(unsigned int j = 0; j < table_ptr->rows[i]->columns; j++){
            printf("%s",table_ptr->rows[i]->cells[j]->cell);
            if(table_ptr->rows[i]->columns != j+1){
                printf("%c",delim[0]);    
            }
        }   
        printf("\n");     
    }
    return 0;
}

//zarovnanie tabulky
int table_alignment(tabulka_t *table_ptr){
    unsigned int max_num_cols = 0; //najvyssi pocet buniek
    for(unsigned int i = 0; i < table_ptr->row_count; i++){  //hladanie najvyssieho poctu buniek
        if(max_num_cols < table_ptr->rows[i]->columns)
            max_num_cols = table_ptr->rows[i]->columns;
    } 
     
    for(unsigned int j = 0; j < table_ptr->row_count; j++){
        int num_of_new_cells = (max_num_cols - table_ptr->rows[j]->columns);
        for(int k = 0; k < num_of_new_cells; k++){
           add_cell_to_row(table_ptr->rows[j]);
        }
    }
    return 0;
}

//uvolnovanie pamate
void free_function(tabulka_t *table_ptr){
    for(unsigned int i = 0; i < table_ptr->row_count; i++){
        for(unsigned int j = 0; j < table_ptr->rows[i]->columns; j++){
            free(table_ptr->rows[i]->cells[j]->cell);
            free(table_ptr->rows[i]->cells[j]);
        }
        free(table_ptr->rows[i]->cells);
        free(table_ptr->rows[i]);
    }
    free(table_ptr->rows);
    free(table_ptr);
}

/************************************************************/
/*                 PRIKAZY PRE ZMENU VYBERU                 */

//spracovanie CMD_SEQUENCE prikazov
int process_CMD_SEQ(char *command, cells_selection *range, tabulka_t *table_ptr){
    if(command[0] == '['){
        selection_procces(command, range, table_ptr);
        table_magnificante(table_ptr, range);
    }
    else if(strchr(command, ' ') != NULL || strcmp(command,"clear") == 0)
        cell_edit(command, range, table_ptr);
    else
        struct_edit(command, range, table_ptr);

    return 0;
}

//delenie CMD_SEQ.
int CUT_CMD_SEQ(int argc, char *argv[], tabulka_t *table_ptr){
    char *CMD_SEQ_pos;
    (void)argc;
    (void)table_ptr;
    if (strcmp(argv[1], "-d") == 0)
        CMD_SEQ_pos = argv[3];
    else
        CMD_SEQ_pos = argv[1];

    char command[1001] = {0};
    int j = 0;
    cells_selection range = {0};
    for(unsigned int i = 0; i < (unsigned int)strlen(CMD_SEQ_pos)+1; i++){
        if(CMD_SEQ_pos[i] == ';' || CMD_SEQ_pos[i] == '\0'){
            command[j] = '\0';
            process_CMD_SEQ(command, &range, table_ptr); //zmena vyberu
            j = 0;
            continue;
        }
        command[j] = CMD_SEQ_pos[i];
        j++; 
    }
    
return 0;
}

//pocitanie znakov
int char_count(char *command, char c){
    int count = 0;
    for(int i = 0; i < (int)strlen(command);i++){
        if(command[i] == c){
            count++;    
        }
    }
    return count;
}

//funkcia - selekcia buniek
int selection_procces(char *command, cells_selection *range, tabulka_t *table_ptr){
    int count_char = sscanf(command,"[%d,%d]",&range->rowX1,&range->colY1);
    
    if(count_char && char_count(command, ',') == 1 && char_count(command, '_') == 0){
        range->rowX2 = --(range->rowX1);
        range->colY2 = --(range->colY1);
        return 0;
    }
    
    count_char = sscanf(command,"[%d,_]",&range->rowX1);
    if(count_char && char_count(command, ',') == 1){
        range->rowX2 = --(range->rowX1);
        range->colY1 = 0;
        range->colY2 = table_ptr->rows[0]->columns-1;
        return 0;
    } 
    count_char = sscanf(command,"[_,%d]",&range->colY1);
    if(count_char && char_count(command, ',') == 1){
        range->colY2 = --(range->colY1);
        range->rowX1 = 0;
        range->rowX2 = table_ptr->row_count-1;
        return 0;
    }
    count_char = strcmp(command,"[_,_]");
    if(count_char == 0){
        range->rowX1 = 0;
        range->colY1 = 0;
        range->rowX2 = table_ptr->row_count-1;
        range->colY2 = table_ptr->rows[0]->columns-1;
        return 0;
    }
    count_char = strcmp(command,"[min]");
    if(count_char == 0){
        find_min(table_ptr, range);
        return 0;
    }
    count_char = strcmp(command,"[max]");
    if(count_char == 0){
        find_max(table_ptr, range);
        return 0;
    }
    count_char = strstr(command, "[find ") == command;
    if(count_char == 1){
        find_STR(table_ptr,range,command);
        return 0;
    }
    count_char = sscanf(command,"[%d,%d,%d,%d]",&range->rowX1,&range->colY1,&range->rowX2,&range->colY2);

    range->rowX1--;  //-1, pretoze vstup indexujeme od 1
    range->colY1--;
    range->rowX2--;
    range->colY2--;
    return 0;        
}

//zvacsenie tabulky v pripade ze selekcia je vacsia ako povodna velkost tabulky
int table_magnificante(tabulka_t *table_ptr, cells_selection *range){
    if(table_ptr->row_count < (unsigned)range->rowX1+1 || table_ptr->row_count < (unsigned)range->rowX2+1){
        int rows;
        if(range->rowX1 > range->rowX2)
            rows = range->rowX1;
        else
            rows = range->rowX2;
        
        for(int i = table_ptr->row_count-1; i < rows; i++){
            irow_arow("arow", i, i, table_ptr);
        }       
    }
    if(table_ptr->rows[0]->columns < (unsigned)range->colY1+1 || table_ptr->rows[0]->columns < (unsigned)range->colY2+1){
        int columns;
        if(range->colY1 > range->colY2)
            columns = range->colY1;
        else
            columns = range->colY2;

        for(int i = table_ptr->rows[0]->columns-1; i < columns; i++){
            icol_acol("acol", i, i, table_ptr);
        }    
    }    
    return 0;
}

//najde MIN num.hodnotu z uz existujuceho vyberu buniek a vyber nastavi na nu
int find_min(tabulka_t *table_ptr, cells_selection *range){
    unsigned int min_num = UINT_MAX; 
    int num_col = 0;
    int num_row = 0;
    for(int i = range->rowX1; i <= range->rowX2; i++){
        for(int j = range->colY1; j <= range->colY2; j++){
            int valid = 0;
            float cell_with_num = str_to_num(table_ptr->rows[i]->cells[j], &valid);
            if(valid == 0)
                continue;
            if(min_num > cell_with_num){
                min_num = cell_with_num;
                num_row = i; 
                num_col = j; 
            }  
        }
    }
    range->rowX1 = num_row;
    range->rowX2 = num_row; 
    range->colY1 = num_col;
    range->colY2 = num_col;
    return 0;
}

//najde bunku s maximalnou hodnotou z uz existujuceho vyberu buniek
int find_max(tabulka_t *table_ptr, cells_selection *range){
    unsigned int max_num = 0; 
    int num_col = 0;
    int num_row = 0;
    for(int i = range->rowX1; i <= range->rowX2; i++){
        for(int j = range->colY1; j <= range->colY2; j++){
            int valid = 0;
            float cell_with_num = str_to_num(table_ptr->rows[i]->cells[j], &valid);
            if(valid == 0)
                continue;
            if(max_num < cell_with_num){
                max_num = cell_with_num;
                num_row = i; 
                num_col = j; 
            } 
        }
    }
    range->rowX1 = num_row;
    range->rowX2 = num_row; 
    range->colY1 = num_col;
    range->colY2 = num_col;
    return 0;    
}

//najde bunku, ktora obsahuje podretazec STR z uz existujuceho vyberu buniek
int find_STR(tabulka_t *table_ptr, cells_selection *range, char *command){
    char *ret;
    int found = 0;
    ret = strchr(command, ' ');
    char *STR = ret+1;
    STR[strlen(STR)-1] = '\0';
    for(int i = range->rowX1; i <= range->rowX2 && found == 0; i++){
        for(int j = range->colY1; j <= range->colY2; j++){
            if(strstr(table_ptr->rows[i]->cells[j]->cell, STR)){
                found = 1;
                range->rowX1 = i;
                range->rowX2 = i; 
                range->colY1 = j;
                range->colY2 = j; 
                break; 
            }          
        }  
    }
    return 0;      
}

//prevod stringu na cislo
float str_to_num(cell_t *cell, int *valid){
    char *end_ptr;
    float num = strtof(cell->cell, &end_ptr);
    if(end_ptr[0] == '\0' && strlen(cell->cell) >= 1)
        *valid = 1;
    else
        *valid = 0;
    return num; 
}

/**********************************************************/
/*                 UPRAVA OBSAHU BUNEK                    */  

int cell_edit(char *command, cells_selection *range, tabulka_t *table_ptr){
    int ret = strncmp(command, "set", 3); 
    if(ret == 0){
        set_str(table_ptr, range, command);
    }
    ret = strcmp(command,"clear");
    if(ret == 0){
        cell_clear(table_ptr, range);
    }
    ret = strncmp(command, "swap", 4);
    if(ret == 0){
        swap(table_ptr, range, command);
    }
    ret = strncmp(command, "sum", 3);
    if(ret == 0){
        sum(table_ptr, range, command);
    }
    ret = strncmp(command, "avg", 3);
    if(ret == 0){
        avg(table_ptr, range, command);    
    }
    ret = strncmp(command, "count", 5);
    if(ret == 0){
        count(table_ptr, range, command);
    }
    ret = strncmp(command, "len", 3);
    if(ret == 0){
        len(table_ptr, range, command);
    }

    return 0;
}

//nastavi hodnotu bunky na STR
int set_str(tabulka_t *table_ptr, cells_selection *range, char *command){
    char *str_start;
    str_start = strchr(command, ' ');
    str_start++;
    str_start[strlen(str_start)] = '\0';
    for(int i = range->rowX1; i <= range->rowX2; i++){
        for(int j = range->colY1; j <= range->colY2; j++){
            char *new_cell;
            new_cell = (char *) realloc(table_ptr->rows[i]->cells[j]->cell, strlen(str_start) * sizeof(char) + 1);
            if(new_cell == NULL){
                fprintf(stderr,"ERROR\n");
                return -1;   
            }
            table_ptr->rows[i]->cells[j]->cell = new_cell;
            strcpy(new_cell,str_start);
        } 
    }
    return 0;
}

//vymaze obsah bunky
int cell_clear(tabulka_t *table_ptr, cells_selection *range){
    for(int i = range->rowX1; i <= range->rowX2; i++){
        for(int j = range->colY1; j <= range->colY2; j++){
            char *new_cell;
            new_cell = (char *) realloc(table_ptr->rows[i]->cells[j]->cell, sizeof(char));
            if(new_cell == NULL){
                fprintf(stderr,"ERROR\n");
                return -1;   
            }
            table_ptr->rows[i]->cells[j]->cell = new_cell;
            new_cell[0] = '\0';
        } 
    }
    return 0;
}

//zameni obsah vybranej bunky
int swap(tabulka_t *table_ptr, cells_selection *range, char *command){
    char *ret;
    ret = strchr(command, ' ');
    ret++;
    cells_selection cell_to_swap = {0};
    int count_char = sscanf(ret,"[%d,%d]",&cell_to_swap.rowX1,&cell_to_swap.colY1);
    if(count_char >= 2){
        cell_to_swap.rowX1--;
        cell_to_swap.colY1--;
    }
    else{
        fprintf(stderr,"CHYBA\n");
        return -1;
    }
    for(int i = range->rowX1; i <= range->rowX2; i++){
        for(int j = range->colY1; j <= range->colY2; j++){
            char *temp_cell;
            temp_cell = table_ptr->rows[i]->cells[j]->cell;
            table_ptr->rows[i]->cells[j]->cell = table_ptr->rows[cell_to_swap.rowX1]->cells[cell_to_swap.colY1]->cell;
            table_ptr->rows[cell_to_swap.rowX1]->cells[cell_to_swap.colY1]->cell = temp_cell;
        }
    }
    return 0;                
}

//do bunky ulozi sucet vybranych buniek
int sum(tabulka_t *table_ptr, cells_selection *range, char *command){
    char *ret;
    ret = strchr(command, ' ');
    ret++;
    cells_selection cell = {0};
    int count_char = sscanf(ret,"[%d,%d]",&cell.rowX1,&cell.colY1);
    if(count_char >= 2){
        cell.rowX1--;
        cell.colY1--;
    }
    else{
        fprintf(stderr,"CHYBA\n");
        return -1;
    }
    float result = 0;
    int valid = 0;
    for(int i = range->rowX1; i <= range->rowX2; i++){
        for(int j = range->colY1; j <= range->colY2; j++){
            result += str_to_num(table_ptr->rows[i]->cells[j], &valid);
        }
    }
    int num_of_digits = snprintf(NULL, 0, "%g", result);
    char *cell_w_res;
    cell_w_res = (char *) realloc(table_ptr->rows[cell.rowX1]->cells[cell.colY1]->cell, sizeof(char) * num_of_digits + 1);
    if(cell_w_res == NULL){
        fprintf(stderr,"ERROR\n");
        return -1;   
    }
    sprintf(cell_w_res, "%g", result); 
    table_ptr->rows[cell.rowX1]->cells[cell.colY1]->cell = cell_w_res;
    return 0;      
}

//do bunky ulozi aritmeticky priemer vybranych buniek
int avg(tabulka_t *table_ptr, cells_selection *range, char *command){
    char *ret;
    ret = strchr(command, ' ');
    ret++;
    cells_selection cell = {0};
    int count_char = sscanf(ret,"[%d,%d]",&cell.rowX1,&cell.colY1);
    if(count_char >= 2){
        cell.rowX1--;
        cell.colY1--;
    }
    else{
        fprintf(stderr,"CHYBA\n");
        return -1;
    }
    float result = 0;
    int valid = 0;
    int num_of_num = 0;
    for(int i = range->rowX1; i <= range->rowX2; i++){
        for(int j = range->colY1; j <= range->colY2; j++){
            result += str_to_num(table_ptr->rows[i]->cells[j], &valid);
            if(valid == 1)
                num_of_num++;
        }
    }
    result = (result / num_of_num);
    int num_of_digits = snprintf(NULL, 0, "%g", result);
    char *cell_w_res;
    cell_w_res = (char *) realloc(table_ptr->rows[cell.rowX1]->cells[cell.colY1]->cell, sizeof(char) * num_of_digits + 1);
    
    if(cell_w_res == NULL){
        fprintf(stderr,"ERROR\n");
        return -1;   
    }
    sprintf(cell_w_res, "%g", result); 
    table_ptr->rows[cell.rowX1]->cells[cell.colY1]->cell = cell_w_res;
    return 0;      
}

//do bunky ulozi pocet neprazdnych buniek
int count(tabulka_t *table_ptr, cells_selection *range, char *command){
    char *ret;
    ret = strchr(command, ' ');
    ret++;
    cells_selection cell = {0};
    int count_char = sscanf(ret,"[%d,%d]",&cell.rowX1,&cell.colY1);
    
    if(count_char >= 2){
        cell.rowX1--;
        cell.colY1--;
    }
    else{
        fprintf(stderr,"CHYBA\n");
        return -1;
    }
    float result = 0;
    
    for(int i = range->rowX1; i <= range->rowX2; i++){
        for(int j = range->colY1; j <= range->colY2; j++){
            if(strlen(table_ptr->rows[i]->cells[j]->cell) != 0)
                result++; 
        }
    }
    int num_of_digits = snprintf(NULL, 0, "%g", result);
    char *cell_w_res;
    cell_w_res = (char *) realloc(table_ptr->rows[cell.rowX1]->cells[cell.colY1]->cell, sizeof(char) * num_of_digits + 1);
    
    if(cell_w_res == NULL){
        fprintf(stderr,"ERROR\n");
        return -1;   
    }
    sprintf(cell_w_res, "%g", result); 
    table_ptr->rows[cell.rowX1]->cells[cell.colY1]->cell = cell_w_res;
    return 0;      
}

//do bunky ulozi dlzku retazca aktualnej bunky
int len(tabulka_t *table_ptr, cells_selection *range, char *command){
    char *ret;
    ret = strchr(command, ' ');
    ret++;
    cells_selection cell = {0};
    int count_char = sscanf(ret,"[%d,%d]",&cell.rowX1,&cell.colY1);
    
    if(count_char >= 2){
        cell.rowX1--;
        cell.colY1--;
    }
    else{
        fprintf(stderr,"CHYBA\n");
        return -1;
    }
    float result = 0;
    
    for(int i = range->rowX1; i <= range->rowX2; i++){
        for(int j = range->colY1; j <= range->colY2; j++){
            result = strlen(table_ptr->rows[i]->cells[j]->cell);
        }
    }
    int num_of_digits = snprintf(NULL, 0, "%g", result);
    char *cell_w_res;
    cell_w_res = (char *) realloc(table_ptr->rows[cell.rowX1]->cells[cell.colY1]->cell, sizeof(char) * num_of_digits + 1);
    
    if(cell_w_res == NULL){
        fprintf(stderr,"ERROR\n");
        return -1;   
    }
    sprintf(cell_w_res, "%g", result); 
    table_ptr->rows[cell.rowX1]->cells[cell.colY1]->cell = cell_w_res;
    return 0;          
}

/**********************************************************/
/*              UPRAVA STRUKTURY TABULKY                  */

int struct_edit(char *command, cells_selection *range, tabulka_t *table_ptr){
    int ret = strcmp(command, "irow") == 0 || strcmp(command, "arow") == 0; 
    
    if(ret == 1){
        irow_arow(command, range->rowX1, range->rowX2, table_ptr);
    }
    ret = strcmp(command, "icol") == 0 || strcmp(command, "acol") == 0;
    
    if(ret == 1){
        icol_acol(command, range->colY1, range->colY2, table_ptr);
    }
    return 0;    
}

//vlozi novy riadok nad alebo pod vybranu bunku
int irow_arow(char *command, int rowX1, int rowX2, tabulka_t *table_ptr){
    add_new_line(table_ptr); 
    row_t *last_line = table_ptr->rows[table_ptr->row_count-1];  
    
    for(unsigned int i = 0; i < table_ptr->rows[0]->columns; i++){
        add_cell_to_row(last_line);    
    }
    int line_index;
    
    if(strcmp(command, "irow") == 0)
        line_index = rowX1;            
    else
        line_index = rowX2+1;
    
    int lines_to_move = table_ptr->row_count-1 - line_index;
    
    memmove(&table_ptr->rows[line_index+1], &table_ptr->rows[line_index], sizeof(row_t*) * lines_to_move);
    table_ptr->rows[line_index] = last_line;
    
    return 0;
}

//vlozi novy stlpec pred alebo za vybranu bunku
int icol_acol(char *command, int colY1, int colY2, tabulka_t *table_ptr){
    int cell_index;
    for(unsigned int i = 0; i < table_ptr->row_count; i++){
        add_cell_to_row(table_ptr->rows[i]);
        
        if(strcmp(command, "icol") == 0)
            cell_index = colY1;
        else
            cell_index = colY2+1;
        
        int cells_to_move = table_ptr->rows[i]->columns-1 - cell_index;
        int i_last_cell = table_ptr->rows[i]->columns;
        cell_t *new_cell = table_ptr->rows[i]->cells[i_last_cell-1];

        memmove(&table_ptr->rows[i]->cells[cell_index+1], &table_ptr->rows[i]->cells[cell_index], sizeof(cell_t*) * cells_to_move);
        table_ptr->rows[i]->cells[cell_index] = new_cell;
    }
    return 0;
}

//ulozi upravenu tabulku do povodneho suboru
int insert_to_file(tabulka_t *table_ptr, char delim, char *filename){
    FILE *fp;
    (void)delim;
    fp = fopen(filename,"w");
    for(unsigned int i = 0; i < table_ptr->row_count; i++){
        for(unsigned int j = 0; j < table_ptr->rows[i]->columns; j++){
            fprintf(fp, "%s", table_ptr->rows[i]->cells[j]->cell);
            if(table_ptr->rows[i]->columns != j+1){
                fprintf(fp,"%c",delim);    
            }
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    return 0;
}