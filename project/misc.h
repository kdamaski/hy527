void push_to_free(struct u_thread *th, struct u_thread *free_list);

void ready_enqueue(struct u_thread *th, struct th_ready_q *ready_q);

struct u_thread *ready_dequeue(struct th_ready_q *ready_q);

void print_Q(struct th_ready_q *ready_q, struct u_thread *curr_thr);

void empty_free(struct u_thread *free_list);

void push_to_join(struct u_thread *th, struct u_thread *curr_thr);
