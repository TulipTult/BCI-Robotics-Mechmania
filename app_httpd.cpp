// Add this to your existing app_httpd.cpp file
// Add the function declaration at the top with other external functions:

extern void register_cmd_uri(httpd_handle_t server);

// Then find the startCameraServer() function and add this line right after 
// the server has been started and before the return statement:

// Existing code creates the server with httpd_start()...
// Add this line after server creation:
register_cmd_uri(server);
// ...rest of the code
