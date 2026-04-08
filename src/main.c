#include "setup.h"
#include "listener.h"


int main(int argc, char *argv[]){

    // initialize configuration informations
    config_infos cfg_infos;
    memset(&cfg_infos, 0, sizeof(config_infos));
    cfg_infos.quiet = false;
    cfg_infos.verbose = false;

    for (int i = 0; i < argc; i++){
        if(strcmp(argv[i], "-v") == 0) cfg_infos.verbose = true;
        if(strcmp(argv[i], "-q") == 0) cfg_infos.quiet = true;
    }
    if(cfg_infos.quiet && cfg_infos.verbose){
        cfg_infos.quiet = false;
        cfg_infos.verbose = false;
    }

    // run server 
    int sock_fd = setup_server();
    if (sock_fd == -1){
        perror("server : socket setup error ");
        return 1;
    }

    listener(&cfg_infos, sock_fd);

    return 0;
}