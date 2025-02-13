#ifndef CONFIG_H
#define CONFIG_H

#define PORT 80

typedef struct {
    char *route;
    char *filename;
} Route;

Route ROUTES[] = {
    {"/", "index.html"},
    {"/contact", "contact.html"},
    {NULL, NULL}  
};

#endif
