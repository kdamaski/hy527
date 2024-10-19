
struct damthread *thr_dequeue();

void thr_enqueue(struct damthread *th);

void print_Q();

void insert_to_free(struct damthread *th);

void empty_free();

struct damthread *thread_exists(unsigned tid);
