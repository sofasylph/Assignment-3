/*
 * Functions used by Communications library
 */
int frontend_setup(void);
int frontend_receive_from_backend(char* buf, int bufsize);
int frontend_send_to_backend(char* buf, int bufsize);
int frontend_close(void);

int backend_setup(void);
int backend_receive_from_frontend(char* buf, int bufsize);
int backend_send_to_frontend(char* buf, int bufsize);
int backend_close(void);
