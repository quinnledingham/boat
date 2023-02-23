#ifndef STRING_H
#define STRING_H

function u32
get_length(const char *string)
{
    if (string == 0)
        return 0;
    
    u32 length = 0;
    const char *ptr = string;
    while(*ptr != 0)
    {
        length++;
        ptr++;
    }
    return length;
}

// copys string into memory
function const char*
copy(const char *string)
{
    u32 length = get_length(string);
    if (length == 0)
        return 0;
    
    char *new_string = (char*)SDL_malloc(length + 1);
    SDL_memset(new_string, 0, length + 1);
    SDL_memcpy(new_string, string, length);
    return new_string;
}

#endif //STRING_H
