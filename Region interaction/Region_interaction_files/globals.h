
#define SQR(x) ((x)*(x))	/* macro for square */
#ifndef M_PI			/* in case M_PI not found in math.h */
#define M_PI 3.1415927
#endif
#ifndef M_E
#define M_E 2.718282
#endif

#define MAX_FILENAME_CHARS	320

char	filename[MAX_FILENAME_CHARS];

HWND	MainWnd;

		// Display flags
int		ShowPixelCoords;
int		Play;
int		Step;
int		Refresh;
int		Predicate;
int		Color;

		// Image data
unsigned char	*OriginalImage;
int				ROWS,COLS;
int				xp,yp;
int				red;
int				blue;
int				green;

		//Predicates
int		absd;
int		ctrd;

#define TIMER_SECOND	1			/* ID of timer used for animation */

		// Drawing flags
int		TimerRow,TimerCol;
int		ThreadRow,ThreadCol;
int		ThreadRunning;
int		SetEnable;

		// Function prototypes
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
void PaintImage();
void AnimationThread(void *);		/* passes address of window */
void region(void *);		/* passes address of window */
void RegionGrow();