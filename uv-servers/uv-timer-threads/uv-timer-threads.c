#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <uv.h>

void on_after_work(uv_work_t* req, int status) {
    free(req);
}

void on_work(uv_work_t* req) {
    if (rand() % 5 == 0) {
        printf("Sleeping...\n");
        sleep(3);
    }
}

void on_timer(uv_timer_t* timer) {
    uint64_t timestamp = uv_hrtime();
    printf("on_timer [%" PRIu64 " ms]\n", (timestamp / 1000000) % 100000);

    // This creates a threadpool to enqueue the task and allows the callee to return immediately.
    uv_work_t* work_req = (uv_work_t*)malloc(sizeof(*work_req));
    // Initializes work_req which will run the on_work() function in a thread from the threadpool.
    // Once the on_work() is completed, on_after_work is invoked on the main loop.
    uv_queue_work(uv_default_loop(), work_req, on_work, on_after_work);
}

int main(int argc, const char** argv) {
    uv_timer_t timer;
    uv_timer_init(uv_default_loop(), &timer);
    uv_timer_start(&timer, on_timer, 0, 1000);
    return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}