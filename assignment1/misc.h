void push_to_join(struct u_thread *th);
// struct u_thread *pop_from_join(struct u_thread *jlist);

struct u_thread *ready_dequeue();

void ready_enqueue(struct u_thread *th);

void print_Q();

void push_to_free(struct u_thread *th);
void empty_free();
