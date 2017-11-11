#include "utils.h"
#include "command.h"
#include "chat.h"

int main(int argc, char **argv) {


    if (argc != 2) {
        fprintf(stderr, "usage: server port\n");
        exit(1);
    }

    init_chat();

    // create thread for the commands from the clients
    struct thread_arg targ;
    targ.thread_num = 0;
    strncpy(targ.port, argv[1], sizeof(targ.port));
    int ret = pthread_create(
            &targ.thread_id,
            NULL,
            command_handler,
            &targ);
    if (ret != 0) handle_error_en(ret, "pthread_create");

    // wait for the command thread
    void* ret_value;
    pthread_join(targ.thread_id, &ret_value);
    printf("joined with thread %d; returned value was %s\n",
            targ.thread_num, (char*)ret_value);
    free(ret_value);

    final_chat();

    return 0;
}
