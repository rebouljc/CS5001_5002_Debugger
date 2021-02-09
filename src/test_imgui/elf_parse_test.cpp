#include<stdio.h>
#include<fcntl.h> // open, read
#include<unistd.h>
#include<elf.h>


int main(){
    void* buffer[1024];
    printf("hello\n");
    int fd = open("a.out", O_RDONLY);
    printf("fd: %i\n", fd);
    ssize_t bytes_read = read(fd, buffer, 1024);
    printf("bytes read: %li\n", bytes_read);

    char elf_desc[4] = {'\0'};
    for(int i = 0; i < 3; i++){
        elf_desc[i] = ((char*)buffer)[i+1];
    }

    if(((char*)buffer)[4] == ELFCLASS64){
        printf("64 bit, %i, %i\n", ELFCLASS64, ELFCLASS32);
    }else if(((char*)buffer)[4] == ELFCLASS32){
        printf("32 bit\n");
    }


    printf("%s\n", elf_desc);
}
