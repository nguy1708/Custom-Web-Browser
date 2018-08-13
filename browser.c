#include "wrapper.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_TAB 100
extern int errno;

/*
 * Name:                uri_entered_cb
 * Input arguments:     'entry'-address bar where the url was entered
 *                      'data'-auxiliary data sent along with the event
 * Output arguments:    none
 * Function:            When the user hits the enter after entering the url
 *                      in the address bar, 'activate' event is generated
 *                      for the Widget Entry, for which 'uri_entered_cb'
 *                      callback is called. Controller-tab captures this event
 *                      and sends the browsing request to the ROUTER (parent)
 *                      process.
 */
void uri_entered_cb(GtkWidget* entry, gpointer data) {
	printf("Attempt to enter a url\n");
	if(data == NULL) {
		return;
	}
	browser_window *b_window = (browser_window *)data;
	// This channel have pipes to communicate with ROUTER.
	comm_channel channel = ((browser_window*)data)->channel;
	// Get the tab index where the URL is to be rendered
	int tab_index = query_tab_id_for_request(entry, data);
	if (tab_index <= 0) {
		fprintf(stderr, "Invalid tab index (%d).", tab_index);
		return;
	}
	// Get the URL.
	char * uri = get_entered_uri(entry);
	// Append your code here
	// Send 'which' message to 'which' process?
	//

	child_req_to_parent url;
	url.type = NEW_URI_ENTERED;
	url.req.uri_req.render_in_tab = tab_index;
	strcpy(url.req.uri_req.uri, uri);
	int writerror = write(channel.child_to_parent_fd[1], &url, sizeof(child_req_to_parent));
	if (writerror == -1){
		printf("ERROR in writing uri_entered_cb\n");
	}
}

/*
 * Name:                create_new_tab_cb
 * Input arguments:     'button' - whose click generated this callback
 *                      'data' - auxillary data passed along for handling
 *                      this event.
 * Output arguments:    none
 * Function:            This is the callback function for the 'create_new_tab'
 *                      event which is generated when the user clicks the '+'
 *                      button in the controller-tab. The controller-tab
 *                      redirects the request to the ROUTER (parent) process
 *                      which then creates a new child process for creating
 *                      and managing this new tab.
 */
void create_new_tab_cb(GtkButton *button, gpointer data)
{
	printf("Attempt to create a new tab.\n");
	if(data == NULL) {
		return;
	}
	//browser_window *b_window = (browser_window *)data;
	// This channel have pipes to communicate with ROUTER.
	comm_channel channel = ((browser_window*)data)->channel;
	// Append your code here
	// Send 'which' message to 'which' process?
	//
	child_req_to_parent new_tab;
	new_tab.type = CREATE_TAB;
	int writerror = write(channel.child_to_parent_fd[1], &new_tab, sizeof(child_req_to_parent));
	if (writerror == -1){
		printf("ERROR in writing create_new_tab_cb\n");
	}
}

/*
 * Name:                url_rendering_process
 * Input arguments:     'tab_index': URL-RENDERING tab index
 *                      'channel': Includes pipes to communctaion with
 *                      Router process
 * Output arguments:    none
 * Function:            This function will make a URL-RENDRERING tab Note.
 *                      You need to use below functions to handle tab event.
 *                      1. process_all_gtk_events();
 *                      2. process_single_gtk_event();
*/
int url_rendering_process(int tab_index, comm_channel *channel) {
	printf("Rendering new url process.");
	browser_window * b_window = NULL;
	// Create url-rendering window
	create_browser(URL_RENDERING_TAB, tab_index, NULL, NULL, &b_window, channel);
	child_req_to_parent req;
	while (1) {  // ERROR HERE?????
		// Append your code here
		// Handle 'which' messages?
		//
		process_single_gtk_event();

		int read_status;
		read_status = read(channel->parent_to_child_fd[0], &req, sizeof(child_req_to_parent));
	  if ((read_status == - 1) && (errno == EAGAIN)){
			usleep(1000);
		}
		else{
		  switch (req.type) {
				case TAB_KILLED:
			    process_all_gtk_events();
			    // free(channel);   ???????
			    exit(0);
			    break;

			  case NEW_URI_ENTERED:
			    render_web_page_in_tab(req.req.uri_req.uri, b_window);
					break;

			  case CREATE_TAB:
			    fprintf(stderr, "Error: New tab was called inside rendering process at %d\n", tab_index);

			  default:
					break;
			  }
			}
		}
	return 0;
}
/*
 * Name:                controller_process
 * Input arguments:     'channel': Includes pipes to communctaion with
 *                      Router process
 * Output arguments:    none
 * Function:            This function will make a CONTROLLER window and
 *                      be blocked until the program terminates.
 */
int controller_process(comm_channel *channel) {
	// Do not need to change code in this function
	close(channel->child_to_parent_fd[0]);
	close(channel->parent_to_child_fd[1]);
	browser_window * b_window = NULL;
	// Create controler window
	create_browser(CONTROLLER_TAB, 0, G_CALLBACK(create_new_tab_cb), G_CALLBACK(uri_entered_cb), &b_window, channel);
	show_browser();
	return 0;
}

/*
 * Name:                router_process
 * Input arguments:     none
 * Output arguments:    none
 * Function:            This function will make a CONTROLLER window and be blocked until the program terminate.
 */
int router_process() {
	comm_channel *channel[MAX_TAB];
	// Append your code here
	// [DONE] Prepare communication pipes with the CONTROLLER process
	// [DONE] Fork the CONTROLLER process
	// [DONE]       call controller_process() in the forked CONTROLLER process
	// [DONE]Poll child processes' communication channels using non-blocking pipes.
	//       handle received messages:
	//          CREATE_TAB:
	// [DONE]      Prepare communication pipes with a new URL-RENDERING process
	// [DONE]      Fork the new URL-RENDERING process
	//          NEW_URI_ENTERED:
	// [DONE]      Send message to the URL-RENDERING process in which the new url is going to be rendered
	//          TAB_KILLED:
	// [DONE]      If the killed process is the CONTROLLER process, send messages to kill all URL-RENDERING processes
	// [DONE]      If the killed process is a URL-RENDERING process, send message to the URL-RENDERING to kill it
	// [DONE]  sleep some time if no message received
	//

	//Process id's to keep track of children (FOR DEBUG PURPOSES)
	//int router = getpid();
	int i;
	int read_status;
	//create an array of the forked processes to keep track of the children processes
	int tab_number[MAX_TAB] = {0};

	// FOR CONTROLLER:
	channel[0] = (comm_channel *) malloc(sizeof(comm_channel));
	if (pipe(channel[0]->parent_to_child_fd) == -1) {
	  printf("Error, pipe fail at router_process\n");
	  return -1;
	}
	if (pipe(channel[0]->child_to_parent_fd) == -1) {
	  printf("Error, pipe fail at router_process\n");
	  return -1;
	}

	//fcntl(channel[0]->child_to_parent_fd[0], F_SETFL, fcntl(channel[0]->parent_to_child_fd[0],F_GETFL) | O_NONBLOCK);
	//Fork controller process
  int pid = fork();
	if (pid == -1) { //ERORR HERE?????
	  perror("Could not Fork in router_process");
	  return -1;
	}
  else if (pid > 0) {
		close(channel[0]->parent_to_child_fd[0]); //READ
		close(channel[0]->child_to_parent_fd[1]); //WRITE
	  //We want to keep track of the children, so we add it to the array
		tab_number[0] = pid;

		fcntl(channel[0]->child_to_parent_fd[0], F_SETFL, fcntl(channel[0]->parent_to_child_fd[0],F_GETFL) | O_NONBLOCK);

		child_req_to_parent child_req;
		while (1) {
		  for (i = 0; i < MAX_TAB; i++) {
		    if (tab_number[i] != 0) {
		      read_status = read(channel[i]->child_to_parent_fd[0], &child_req, sizeof(child_req_to_parent));
					//printf("Value of read status is %d\n", read_status);
					if ((read_status == -1) && (errno == EAGAIN)){
						usleep(1000);
					}
					else if (read_status > 0) {
						switch (child_req.type) {
							case TAB_KILLED:
								printf("Recieved message to kill tab.\n");
							  // ;
							  // int killmsg;
								// killmsg = child_req.req.killed_req.tab_index;
								if (i == 0) { // for controller
								  int j;
									//We want all of the child process to terminate before the controller
									for (j = 1; j < MAX_TAB; j++ ) {
										if (tab_number[j] !=0){

											int writerror;
											writerror = write(channel[j]->parent_to_child_fd[1], &child_req, sizeof(child_req_to_parent));
											if (writerror == -1){
												printf("Error at TAB_KILLED for controller at router_process\n");
											}
											int status;
											waitpid(tab_number[j], &status, WNOHANG);
											close(channel[j]->child_to_parent_fd[0]);
											close(channel[j]->parent_to_child_fd[1]);
											tab_number[j] = 0;
										}
										// int writerror;
										// writerror = write(channel[j]->parent_to_child_fd[1], &child_req, sizeof(child_req_to_parent));
										// if (writerror == -1){
										// 	printf("Error at TAB_KILLED for controller at router_process\n");
										// }

										// waitpid(tab_number[j], NULL, 0);
										// free(channel[j]);
										}
										//After all tabs are terminated, controller can finally terminate
										// write(channel[0]->parent_to_child_fd[1], &child_req, sizeof(child_req_to_parent));
										// waitpid(tab_number[0], NULL, 0);
										// free(channel[0]);
										// exit(0); // NOT SURE
									}
								else { // for specific child
									int writerror;
									writerror = write(channel[i]->parent_to_child_fd[1], &child_req, sizeof(child_req_to_parent));
									if (writerror == -1){
										printf("Error at TAB_KILLED for child at router_process\n");
									}
									tab_number[i] = 0;
									int status = 0;
									waitpid(tab_number[i], &status, WNOHANG);
									close(channel[0]->parent_to_child_fd[1]); //Hoang: Now that I think about it, if channel is freed, do we need to close the pipes?
									close(channel[0]->child_to_parent_fd[0]);
									//free(channel[killmsg]);
									tab_number[i] = 0; //We need to set value to 0 after tab is terminated
									//break;
								}

							case NEW_URI_ENTERED:
								printf("Recieved message of entered url.\n");
							  ;
								int nurl = child_req.req.uri_req.render_in_tab;
								if (nurl <= 0 || nurl >= MAX_TAB){
									printf("Out of bounds tab index");
								} else
								if (tab_number[nurl] != 0) {
									int writerror;
								  writerror = write(channel[nurl]->parent_to_child_fd[1], &child_req, sizeof(child_req_to_parent));
									if (writerror == -1){
										printf("Error at NEW_URI_ENTERED at router_process\n");
									}
								}
								else {  // if tab has not been created, prompt error msg
								  fprintf(stderr, "Cannot load %s at tab %d\n", child_req.req.uri_req.uri, nurl);
								}
								break;


							case CREATE_TAB:
								printf("Recieved message to create new tab\n");
							  ;
							// first find an unused space in tab array
							  int t;
								for (t = 1; t < MAX_TAB; t++){
									if (tab_number[t] == 0){
										break;
									}
								}
								if (t == MAX_TAB){
									printf("Exceed max tab number\n");
								} else {

									channel[t] = (comm_channel *) malloc(sizeof(comm_channel));
									//pipe_status = pipe(channel[t]->parent_to_child_fd);
						      if (pipe(channel[t]->parent_to_child_fd) == -1) {
									  printf("Error, pipe fail at CREATE_TAB in router_process\n");
									}

									if (pipe(channel[t]->child_to_parent_fd) == -1) {
									  printf("Error, pipe fail at CREATE_TAB in router_process\n");
									}

									int pid_tab = fork();
									if (pid_tab == -1) {
									  printf("Could not fork at CREATE_TAB in router_process\n");
									}
									else if (pid_tab == 0) {
									  close(channel[t]->parent_to_child_fd[1]);
									  close(channel[t]->child_to_parent_fd[0]);
										fcntl(channel[t]->parent_to_child_fd[0], F_SETFL, fcntl(channel[t]->parent_to_child_fd[0],F_GETFL) | O_NONBLOCK);
										url_rendering_process(t, channel[t]);
									}
									else {
										tab_number[t] = pid_tab;
									  close(channel[t]->parent_to_child_fd[0]);
									  close(channel[t]->child_to_parent_fd[1]);
										fcntl(channel[t]->child_to_parent_fd[0], F_SETFL, fcntl(channel[t]->child_to_parent_fd[0],F_GETFL) | O_NONBLOCK);
									}
								}


							default:
							  break;
						}
		      }
					else if (read_status == 0){
						printf("ERROR at Reading in router_process\n");
						exit(0);
					}
		    }
		  }
		}
	}
	else { // pid == 0 -> controller
	  controller_process(channel[0]);
		//printf("Error at forking in router_process\n");
	}

	return 0;
}


int main() {
	return router_process();
}
