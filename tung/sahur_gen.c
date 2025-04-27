#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <time.h>


#define TMP_BUF_SIZE 8
#define CORDS 2
#define BUF_LEN 128

typedef struct updateMovement{
	GtkWidget* widget; // widget to move
	int dir; // which direction to move
	int dX; // how many pixels in x coordinate to move?
	int dY; // how many pixels in y coordinate to move?
	GdkScreen* screen; // refering this variable to compare current position with screen size
	size_t winWidth;
	size_t winHeight;
};


void get_image_size_str(int *fd, char cordsInfo[CORDS][TMP_BUF_SIZE]) {

	char buffer[BUF_LEN]; // buffer to store x,y data
	size_t size = read(*fd, buffer, BUF_LEN); // read given file
	
	int cord_num = 0; // indicates which coordinate is (0 = x, 1 = y)
	int buffer_counter = 0; // count read characters from the buffer
	for(int i = 0; i < BUF_LEN; i++) {
		if(cord_num > 1) {
			break; // only reads image's x, y coordinate. 
			       // if currently reading more than two lines, break the loop
		}
		if (buffer[i] == '\n') { // if program reached line break, 
			cord_num++; // increase coordinate counter 
			cordsInfo[cord_num][buffer_counter] = '\0'; // insert terminal null
			buffer_counter = 0; // reset the buffer counter
		}
		else {
			if(buffer[i] > 47 || buffer[i] < 58) { // if read character is number
				cordsInfo[cord_num][buffer_counter] = buffer[i]; // store as coordinate
				buffer_counter++; // increase buffer counter
			}
		}
	}
	close(*fd); // close file descriptor for temporary file

}

void get_image_size(size_t* x, size_t* y) {

	char cordInfo[CORDS][TMP_BUF_SIZE]; // container which stores width and height of background image
	
	pid_t pid = fork(); // creates child process
	if(pid < 0) { // if fork() returns -1, terminate program
		perror("fork() error\n");
		exit(-1);
	}
	else if(pid == 0) { // otherwise, if current process is child process,
		system("touch temp.txt"); // create temporary file to store size data
		system("python3 get_image_size.py | cat >> temp.txt"); // execute python script which take image's width and height, pipeline it to cat command which saves it to temp file
		exit(0); // terminates child process here. If not, it will run the remaining code, and would cause issues
	}
	else { // otherwise, if current process is parent process,
		wait(0); // wait for child process to be done
		int fd = open("temp.txt", O_RDONLY); // open temp.txt file created by child process to acquire width and height of the image
		get_image_size_str(&fd, cordInfo); // this function will take data from the text file, and save into the buffer
		system("rm temp.txt"); // removing temporary file

		*x = atoi(cordInfo[0]); // save width to variable x
		*y = atoi(cordInfo[1]); // save height to variable y
	}
}

// callback function for draw signal
// gtk calls widget which needs to be drawn
// used for drawing background image
// GtkWidget * widget -> drawing area 
// cairo_t *cr -> cairo context variable, used to draw shapes, images, text
// gpointer data -> optional data (not used for this), interesting point is, gpointer is generic pointer so it can be any type of pointer provided by gtk
static gboolean on_draw(GtkWidget* widget, cairo_t *cr, gpointer data) {

	// load image from secondary storage to memory
	// variable pixbuf contains image data
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("images/sahur.png", NULL);

	if(pixbuf) {  // if image loaded successfully
		gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0); // cairo context uses pixbuf data as source for drawing
		// drawing starts from coordinate top-left(0,0)
		cairo_paint(cr); // paints the image onto drawing area based on the source -> image 
		g_object_unref(pixbuf); // freeing the memory when operation is done.
	}

	// reason for returning false is to notify
	// that it does not need to be chained with other handlers.
	return FALSE; 
}


void updateDelta(struct updateMovement* data) {
	
	// randomly define object's velocity
	data->dX = rand() % 10 + 1;
	data->dY = rand() % 10 + 1;

	// 0 -> (+, +), 1 -> (+, -), 2 -> (-, -), 3 -> (-, +)
	switch(data->dir){
		case 0:
			if(data->dX < 0) {
				data->dX *= -1;
			}

			if(data->dY < 0) {
				data->dY *= -1;
			}
			break;
		case 1:
			if(data->dX < 0) {
				data->dX *= -1;
			}
			if(data->dY > 0) {
				data->dY *= -1;
			}
			break;
		case 2:
			if(data->dX > 0) {
				data->dX *= -1;
			}

			if(data->dY > 0) {
				data->dY *= -1;
			}
			break;
		case 3:
			if(data->dX > 0) {
				data->dX *= -1;
			}

			if(data->dY < 0) {
				data->dY *= -1;
			}
			break;
		default:
			perror("cannot update movement\n");
			exit(-1);
	}
}
	

// callback function for window movement
// 
static gboolean update_position(gpointer data) {
	char updateFlag = 0;
	struct updateMovement* currentWidget = (struct updateMovement*)data;

	GtkWidget* window = GTK_WINDOW(currentWidget->widget); // creating pointer pointing to widget data passed. 

	// variable hold current x and y coordinate of window
	int currentX, currentY;
	gtk_window_get_position(GTK_WINDOW(currentWidget->widget), &currentX, &currentY); // optain current widget's X and Y coordinate

	// for tested ubuntu machine
	// screen size = 1920, 966
	// image size = 254, 597
	// x = (66, 1666) y = (32, 369), this needs to be calculated 
	// left and top should be 0 but got padding around that pixels
	if(currentX == 66 || currentX == gdk_screen_get_width(currentWidget->screen) - currentWidget->winWidth) {
		currentWidget->dX *= -1;
	}

	if(currentY == 32 || currentY == gdk_screen_get_height(currentWidget->screen) - currentWidget->winHeight) {
		currentWidget->dY *= -1;
	}

	gtk_window_move(GTK_WINDOW(window), currentX + currentWidget->dX, currentY + currentWidget->dY); // moving the window
	

	return TRUE; // this function needs to be chained for every n times. 
}

// callback function GTK calls when the app starts
// GtkApplication* app is application instance
// gpointer user_data is optional extra data
static void activate(GtkApplication* app, gpointer user_data) { 

	srand(time(NULL));
	GtkWidget * window; // main window
	GtkWidget * background; // window which I am using for drawing background image
	window = gtk_application_window_new(app); // create a new window that belongs to application
	
	size_t x; // window width - same as image width
	size_t y; // window height - same as image height
	get_image_size(&x, &y); // get image's width and height

	gtk_window_set_default_size(GTK_WINDOW(window), x, y); // set window's default size to image size
	gtk_window_set_title(GTK_WINDOW(window), ""); // set the title to empty string

	
    // Make the window background transparent
    GdkRGBA color; 
    gdk_rgba_parse(&color, "rgba(0,0,0,0)"); // Transparent background
    gtk_widget_override_background_color(window, GTK_STATE_FLAG_NORMAL, &color); // overrides window's background color. 
	

	// Make the window borderless (remove title bar, and borders)
	gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
	background = gtk_drawing_area_new(); // creating new drawing area for background
	gtk_widget_set_size_request(background, x, y); // make the background size same as the image size
	
	// connects the "draw" signal to on_draw function
	// on_draw() will called when gtk requests to draw
	g_signal_connect(G_OBJECT(background), "draw", G_CALLBACK(on_draw), NULL);

	gtk_container_add(GTK_CONTAINER(window), background); // adding background to main window

	struct updateMovement* move = g_new(struct updateMovement, 1);
	move->widget = window;
	move->dir = random() % 4;
	move->screen = gdk_screen_get_default();
	move->winWidth = x;
	move->winHeight = y;
	updateDelta(move);

	g_timeout_add(50, update_position, move); // call update_position function with widget window for 0.1 second (100 milliseconds)
	gtk_widget_show_all(window);  // make window visible

}

int main(int argc, char* argv[]) {
	
	GtkApplication *app;
	int status;
	
	app = gtk_application_new("tung.tung.sahur", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);

	g_object_unref(app);

	return status;
}
