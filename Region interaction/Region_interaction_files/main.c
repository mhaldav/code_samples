
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/timeb.h>
#include <windows.h>
#include <wingdi.h>
#include <winuser.h>
#include <process.h>	/* needed for multithreading */
#include "resource.h"
#include "globals.h"

#define SQR(x) ((x)*(x))

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
					 LPTSTR lpCmdLine, int nCmdShow)

{
	MSG			msg;
	HWND		hWnd;
	WNDCLASS	wc;

	wc.style=CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc=(WNDPROC)WndProc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hInstance=hInstance;
	wc.hIcon=LoadIcon(hInstance,"ID_PLUS_ICON");
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName="ID_MAIN_MENU";
	wc.lpszClassName="PLUS";

	if (!RegisterClass(&wc))
		return(FALSE);

	hWnd=CreateWindow("PLUS","plus program",
		WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
		CW_USEDEFAULT,0,550,600,NULL,NULL,hInstance,NULL);
	if (!hWnd)
		return(FALSE);

	ShowScrollBar(hWnd,SB_BOTH,FALSE);
	ShowWindow(hWnd,nCmdShow);
	UpdateWindow(hWnd);
	MainWnd=hWnd;

	/* initialising flags and variables */
	ShowPixelCoords=0;
	Play=0;
	Step=0;
	Refresh=0;
	Predicate=0;
	Color=0;
	SetEnable=0;

	red=255;
	blue=0;
	green=0;

	absd=10;
	ctrd=30;

	strcpy(filename,"");
	OriginalImage=NULL;
	ROWS=COLS=0;

	InvalidateRect(hWnd,NULL,TRUE);
	UpdateWindow(hWnd);

	while (GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return(msg.wParam);
}

BOOL CALLBACK AboutDlgProc1(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
	{
	case WM_INITDIALOG:
		SetDlgItemInt(hWnd,IDC_EDIT1,absd,FALSE);
		SetDlgItemInt(hWnd,IDC_EDIT2,ctrd,FALSE);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			BOOL bSuccess;
		case IDOK:
			absd=GetDlgItemInt(hWnd, IDC_EDIT1, &bSuccess, FALSE);
			ctrd=GetDlgItemInt(hWnd, IDC_EDIT2, &bSuccess, FALSE);
			EndDialog(hWnd, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hWnd, IDCANCEL);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

BOOL CALLBACK AboutDlgProc2(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
	{
	case WM_INITDIALOG:			
		SetDlgItemInt(hWnd,IDC_EDIT1,red,FALSE);
		SetDlgItemInt(hWnd,IDC_EDIT2,green,FALSE);
		SetDlgItemInt(hWnd,IDC_EDIT3,blue,FALSE);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			BOOL bSuccess;
		case IDOK:
			red=GetDlgItemInt(hWnd, IDC_EDIT1, &bSuccess, FALSE);
			blue=GetDlgItemInt(hWnd, IDC_EDIT2, &bSuccess, FALSE);
			green=GetDlgItemInt(hWnd, IDC_EDIT3, &bSuccess, FALSE);
			EndDialog(hWnd, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hWnd, IDCANCEL);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK WndProc (HWND hWnd, UINT uMsg,
						  WPARAM wParam, LPARAM lParam)

{
	HMENU				hMenu;
	OPENFILENAME		ofn;
	FILE				*fpt;
	HDC					hDC;
	char				header[320],text[320];
	int					BYTES,xPos,yPos;

	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			/* set flags */
		case ID_SHOWPIXELCOORDS:
			ShowPixelCoords=(ShowPixelCoords+1)%2;
			PaintImage();
			break;
		case ID_REGIONGROW_PLAY:
			Play=(!Play);
			Step=0;
			break;
		case ID_REGIONGROW_STEP:
			Step=(!Step);
			Play=0;
			break;
		case ID_DISPLAY_PREDICATES:
			Predicate=(!Predicate);
			PaintImage();
			break;
		case ID_DISPLAY_COLOR:
			Color=(!Color);
			PaintImage();
			break;
		case ID_DISPLAY_REFRESH:
			Refresh=(!Refresh);
			ShowPixelCoords=0;
			Play=0;
			Step=0;
			hMenu=GetMenu(MainWnd);
			CheckMenuItem(hMenu,ID_SHOWPIXELCOORDS,MF_UNCHECKED);
			CheckMenuItem(hMenu,ID_DISPLAY_REFRESH,MF_UNCHECKED);
			CheckMenuItem(hMenu,ID_REGIONGROW_PLAY,MF_UNCHECKED);
			CheckMenuItem(hMenu,ID_REGIONGROW_STEP,MF_UNCHECKED);
			CheckMenuItem(hMenu,ID_SHOWPIXELCOORDS,MF_UNCHECKED);
			DrawMenuBar(hWnd);		  
			PaintImage();
			break;
		case ID_FILE_LOAD:
			if (OriginalImage != NULL)
			{
				free(OriginalImage);
				OriginalImage=NULL;
			}
			memset(&(ofn),0,sizeof(ofn));
			ofn.lStructSize=sizeof(ofn);
			ofn.lpstrFile=filename;
			filename[0]=0;
			ofn.nMaxFile=MAX_FILENAME_CHARS;
			ofn.Flags=OFN_EXPLORER | OFN_HIDEREADONLY;
			ofn.lpstrFilter = "PPM files\0*.ppm\0All files\0*.*\0\0";
			if (!( GetOpenFileName(&ofn))  ||  filename[0] == '\0')
				break;		/* user cancelled load */
			if ((fpt=fopen(filename,"rb")) == NULL)
			{
				MessageBox(NULL,"Unable to open file",filename,MB_OK | MB_APPLMODAL);
				break;
			}
			fscanf(fpt,"%s %d %d %d",header,&COLS,&ROWS,&BYTES);
			if (strcmp(header,"P5") != 0  ||  BYTES != 255)
			{
				MessageBox(NULL,"Not a PPM (P5 greyscale) image",filename,MB_OK | MB_APPLMODAL);
				fclose(fpt);
				break;
			}
			OriginalImage=(unsigned char *)calloc(ROWS*COLS,1);
			header[0]=fgetc(fpt);	/* whitespace character after header */
			fread(OriginalImage,1,ROWS*COLS,fpt);
			fclose(fpt);
			SetWindowText(hWnd,filename);
			PaintImage();
			break;

		case ID_FILE_QUIT:
			DestroyWindow(hWnd);
			break;
		}
		break;
	case WM_SIZE:		  /* could be used to detect when window size changes */
		PaintImage();
		return(DefWindowProc(hWnd,uMsg,wParam,lParam));
		break;
	case WM_PAINT:
		PaintImage();
		return(DefWindowProc(hWnd,uMsg,wParam,lParam));
		break;
	case WM_LBUTTONDOWN:
		/* selection of pixel to grow region from */
		if (Play||Step == 1)
		{
			xPos=LOWORD(lParam);
			yPos=HIWORD(lParam);
			xp=xPos;
			yp=yPos;
			_beginthread(region,0,MainWnd);
		}	
		return(DefWindowProc(hWnd,uMsg,wParam,lParam));
		break;
	case WM_MOUSEMOVE:
		if (ShowPixelCoords == 1)
		{
			xPos=LOWORD(lParam);
			yPos=HIWORD(lParam);
			if (xPos >= 0  &&  xPos < COLS  &&  yPos >= 0  &&  yPos < ROWS)
			{
				sprintf(text,"%d,%d=>%d     ",xPos,yPos,OriginalImage[yPos*COLS+xPos]);
				hDC=GetDC(MainWnd);
				TextOut(hDC,0,0,text,strlen(text));		/* draw text on the window */
				SetPixel(hDC,xPos,yPos,RGB(255,0,0));	/* color the cursor position red */
				ReleaseDC(MainWnd,hDC);
			}
		}
		return(DefWindowProc(hWnd,uMsg,wParam,lParam));
		break;
	case WM_KEYDOWN:
		////////////////////////////* shortcuts *//////////////////////////////////
		if (wParam == 'k'  ||  wParam == 'K')
			PostMessage(MainWnd,WM_COMMAND,ID_SHOWPIXELCOORDS,0);	  /* show pixel co-ordinates */
		if (wParam == 'p'  ||  wParam == 'P')
			PostMessage(MainWnd,WM_COMMAND,ID_REGIONGROW_PLAY,0);	  /* play mode */
		if (wParam == 'f'  ||  wParam == 'F')
			PostMessage(MainWnd,WM_COMMAND,ID_DISPLAY_REFRESH,0);	  /* refresh */	
		if (wParam == 's'  ||  wParam == 'S')
			PostMessage(MainWnd,WM_COMMAND,ID_REGIONGROW_STEP,0);	  /* step */	
		if (wParam == 'c'  ||  wParam == 'C')
			PostMessage(MainWnd,WM_COMMAND,ID_DISPLAY_COLOR,0);	  /* color selection */	
		if (wParam == 'd'  ||  wParam == 'D')
			PostMessage(MainWnd,WM_COMMAND,ID_DISPLAY_PREDICATES,0);/* predicate selection */
		if (wParam == 'l'  ||  wParam == 'L')
			PostMessage(MainWnd,WM_COMMAND,ID_FILE_LOAD,0);/* predicate selection */
		if (wParam == 'j'  ||  wParam == 'J')
			SetEnable=1;										  /* growing region in steps */
		if (wParam == 'r'  ||  wParam == 'R')
			if((red+1)/255<=1)
			red++;												  /* increasing the pixel redness */
		if (wParam == 'b'  ||  wParam == 'B')
			if((blue+1)/255<=1)
			blue++;												  /* increasing the pixel blueness */
		if (wParam == 'g'  ||  wParam == 'G')
			if((green+1)/255<=1)
			green++;											  /* increasing the pixel greenness */
		if (wParam == 'o'  ||  wParam == 'O')
		{
			red=0;
			blue=0;
			green=0;
		}
	case WM_TIMER:	  /* this event gets triggered every time the timer goes off */
		break;
	case WM_HSCROLL:	  /* this event could be used to change what part of the image to draw */
		PaintImage();	  /* direct PaintImage calls eliminate flicker; the alternative is InvalidateRect(hWnd,NULL,TRUE); UpdateWindow(hWnd); */
		return(DefWindowProc(hWnd,uMsg,wParam,lParam));
		break;
	case WM_VSCROLL:	  /* this event could be used to change what part of the image to draw */
		PaintImage();
		return(DefWindowProc(hWnd,uMsg,wParam,lParam));
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return(DefWindowProc(hWnd,uMsg,wParam,lParam));
		break;
	}

	hMenu=GetMenu(MainWnd);
	if (ShowPixelCoords == 1)
		CheckMenuItem(hMenu,ID_SHOWPIXELCOORDS,MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
	else
		CheckMenuItem(hMenu,ID_SHOWPIXELCOORDS,MF_UNCHECKED);
	if (Play == 1)
		CheckMenuItem(hMenu,ID_REGIONGROW_PLAY,MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
	else
		CheckMenuItem(hMenu,ID_REGIONGROW_PLAY,MF_UNCHECKED);
	if (Step == 1)
		CheckMenuItem(hMenu,ID_REGIONGROW_STEP,MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
	else
		CheckMenuItem(hMenu,ID_REGIONGROW_STEP,MF_UNCHECKED);
	if(Predicate==1)
	{
		DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1), hWnd, AboutDlgProc1);
		Predicate=0;
	}
	if(Color==1)
	{
		DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG2), hWnd, AboutDlgProc2);
		Color=0;
	}

	DrawMenuBar(hWnd);

	return(0L);
}

void PaintImage()

{
	PAINTSTRUCT			Painter;
	HDC					hDC;
	BITMAPINFOHEADER	bm_info_header;
	BITMAPINFO			*bm_info;
	int					i,r,c,DISPLAY_ROWS,DISPLAY_COLS;
	unsigned char		*DisplayImage;

	if (OriginalImage == NULL)
		return;		/* no image to draw */

	/* Windows pads to 4-byte boundaries.  We have to round the size up to 4 in each dimension, filling with black. */
	DISPLAY_ROWS=ROWS;
	DISPLAY_COLS=COLS;
	if (DISPLAY_ROWS % 4 != 0)
		DISPLAY_ROWS=(DISPLAY_ROWS/4+1)*4;
	if (DISPLAY_COLS % 4 != 0)
		DISPLAY_COLS=(DISPLAY_COLS/4+1)*4;
	DisplayImage=(unsigned char *)calloc(DISPLAY_ROWS*DISPLAY_COLS,1);
	for (r=0; r<ROWS; r++)
		for (c=0; c<COLS; c++)
			DisplayImage[r*DISPLAY_COLS+c]=OriginalImage[r*COLS+c];

	BeginPaint(MainWnd,&Painter);
	hDC=GetDC(MainWnd);
	bm_info_header.biSize=sizeof(BITMAPINFOHEADER); 
	bm_info_header.biWidth=DISPLAY_COLS;
	bm_info_header.biHeight=-DISPLAY_ROWS; 
	bm_info_header.biPlanes=1;
	bm_info_header.biBitCount=8; 
	bm_info_header.biCompression=BI_RGB; 
	bm_info_header.biSizeImage=0; 
	bm_info_header.biXPelsPerMeter=0; 
	bm_info_header.biYPelsPerMeter=0;
	bm_info_header.biClrUsed=256;
	bm_info_header.biClrImportant=256;
	bm_info=(BITMAPINFO *)calloc(1,sizeof(BITMAPINFO) + 256*sizeof(RGBQUAD));
	bm_info->bmiHeader=bm_info_header;
	for (i=0; i<256; i++)
	{
		bm_info->bmiColors[i].rgbBlue=bm_info->bmiColors[i].rgbGreen=bm_info->bmiColors[i].rgbRed=i;
		bm_info->bmiColors[i].rgbReserved=0;
	} 

	SetDIBitsToDevice(hDC,0,0,DISPLAY_COLS,DISPLAY_ROWS,0,0,
		0, /* first scan line */
		DISPLAY_ROWS, /* number of scan lines */
		DisplayImage,bm_info,DIB_RGB_COLORS);
	ReleaseDC(MainWnd,hDC);
	EndPaint(MainWnd,&Painter);

	free(DisplayImage);
	free(bm_info);
}

#define SQR(x) ((x)*(x))

/*
** Given an image, a starting point, and a label, this routine
** paint-fills (8-connected) the area with the given new label
** according to the following criteria :
** 1. absolute difference of the pixel intensity to the average 
**	  intensity of pixels alreadyin the region
** 2. distance of the pixel to the centroidof the pixel already 
**	  in the region
*/
void region(HWND AnimationWindowHandle)
{
	HDC hDC;
	unsigned char *labels;	/* segmentation labels */
	int r=yp;
	int c=xp;		/* pixel to paint from */
	int	RegionSize;		
	labels=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
	hDC=GetDC(MainWnd);
	RegionGrow(OriginalImage,labels,ROWS,COLS,r,c,0,255,&RegionSize,hDC);
	ReleaseDC(MainWnd,hDC);
}

#define MAX_QUEUE 10000	/* max perimeter size (pixels) of border wavefront */

void  RegionGrow(unsigned char *image,	/* image data */
				 unsigned char *labels,	/* segmentation labels */
				 int ROWS,int COLS,	/* size of image */
				 int r,int c,		/* pixel to paint from */
				 int paint_over_label,	/* image label to paint over */
				 int new_label,		/* image label for painting */
				 int *count,HDC hDC)		/* output:  count of pixels painted */
{
	int	r2,c2;
	int	queue[MAX_QUEUE],qh,qt;
	int	average,total;	/* average and total intensity in growing region */
	/* variables for centroid */
	int rc=0; 
	int rr=0;
	double rc1=0;
	double rr1=0;
	double area=0;
	double d=0;
	*count=0;
	if (labels[r*COLS+c] != paint_over_label)
		return;
	labels[r*COLS+c]=new_label;
	if(Refresh!=0)
	{
		SetPixel(hDC,c,r,RGB(red,green,blue));
	}
	average=total=(int)image[r*COLS+c];
	queue[0]=r*COLS+c;
	qh=1;	/* queue head */
	qt=0;	/* queue tail */
	(*count)=1;
	while (qt != qh)
	{
		if ((*count)%50 == 0)	/* recalculate average after each 50 pixels join */
		{
			average=total/(*count);
		}
		/*compute centroid co-ordinate values */
		rc1=(double)rc/area;
		rr1=(double)rr/area;
		/* scan through 8 neighbours */
		for (r2=-1; r2<=1; r2++)
			for (c2=-1; c2<=1; c2++)
			{
				if (r2 == 0  &&  c2 == 0)
					continue;
				if ((queue[qt]/COLS+r2) < 0  ||  (queue[qt]/COLS+r2) >= ROWS  ||
					(queue[qt]%COLS+c2) < 0  ||  (queue[qt]%COLS+c2) >= COLS)
					continue;
				if (labels[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]!=paint_over_label)
					continue;
				/* test criteria to join region */
				if (abs((int)(image[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2])
					-average) > absd) /* criterion for absolute difference in the intensities */
				{
					continue;				
				}
				else 
				{
					d=sqrt(SQR(rc1-(queue[qt]%COLS+c2))+SQR(rr1-(queue[qt]/COLS+r2)));
					if (d > ctrd) /* criterion for distance from the centroid of region */
						continue;
				}

				labels[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]=new_label;		
				if(Play==1) /* checking whether in play mode */
				{
					Sleep(1);/* desired growth rate of 1ms*/
					SetPixel(hDC,queue[qt]%COLS+c2,queue[qt]/COLS+r2,RGB(red,green,blue));
				}
				if(Step==1) /* checking whether in step mode */
				{
					while(SetEnable==0 && Play==0) /* wait till 'j' is pressed and play is not enabled */
					{
					}
					SetPixel(hDC,queue[qt]%COLS+c2,queue[qt]/COLS+r2,RGB(red,green,blue));
					SetEnable=0;
				}
				total+=image[(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2];
				/* computation for the centroid */
				rc=rc+queue[qt]%COLS+c2;
				rr=rr+queue[qt]/COLS+r2;
				area++;
				(*count)++;
				queue[qh]=(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2;
				qh=(qh+1)%MAX_QUEUE;
				if (qh == qt)
				{
					printf("Max queue size exceeded\n");
					exit(0);
				}
			}
			qt=(qt+1)%MAX_QUEUE;
	}
}