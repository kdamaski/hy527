#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

void on_read(uv_fs_t *req) {
  if (req->result > 0) {
    printf("Read %ld bytes: %.*s\n", req->result, (int)req->result,
           (char *)req->bufs->base);
  } else if (req->result == 0) {
    printf("EOF reached\n");
  } else {
    fprintf(stderr, "Error reading file: %s\n", uv_strerror((int)req->result));
  }

  // Cleanup
  uv_fs_req_cleanup(req);
  free(req->bufs->base);
  free(req);
}

void on_open(uv_fs_t *req) {
  if (req->result >= 0) {
    int fd = req->result;
    uv_fs_req_cleanup(req);

    uv_fs_t *read_req = malloc(sizeof(uv_fs_t));
    uv_buf_t buf = uv_buf_init(malloc(1024), 1024);

    // Read asynchronously
    uv_fs_read(uv_default_loop(), read_req, fd, &buf, 1, -1, on_read);
  } else {
    fprintf(stderr, "Error opening file: %s\n", uv_strerror((int)req->result));
  }

  free(req);
}

int main() {
  uv_loop_t *loop = uv_default_loop();

  uv_fs_t *open_req = malloc(sizeof(uv_fs_t));
  uv_fs_open(loop, open_req, "example.txt", O_RDONLY, 0, on_open);

  uv_run(loop, UV_RUN_DEFAULT);

  uv_loop_close(loop);
  return 0;
}
