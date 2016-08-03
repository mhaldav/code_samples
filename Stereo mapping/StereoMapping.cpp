/*****************************************************************************************************************************
Name: Mriganka Haldavnekar
CUID: mhaldav
Email: mhaldav@clemson.edu
*****************************************************************************************************************************/

#include <afxwin.h>  // necessary for MFC to work properly
#include "Homework.h"
#include "../../src/blepo.h"
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include <vector>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace blepo;

int main(int argc, const char* argv[], const char* envp[])
{
	// Initialize MFC and return if failure
	HMODULE hModule = ::GetModuleHandle(NULL);
	if (hModule == NULL || !AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
	{
		printf("Fatal Error: MFC initialization failed (hModule = %x)\n", hModule);
		return 1;
	}

	if (argc >=3){
		cout<<"Programme is working"<<endl;
	}
	else{
		cout<<"Wrong number of arguments"<<endl;
	}


	try
	{

		double maxdisparity;										//User defined sigma value
		CString lfilename;										//Entered image by the user
		CString rfilename;										//Entered image by the user
		lfilename=argv[1];										//Reading the name of the image to be loaded
		rfilename=argv[2];										//Reading the name of the image to be loaded
		maxdisparity=atof(argv[3]);									//Reading sigma from the provided arguments

		//Loading original image
		ImgBgr img1b; 											//Declaring BGR image
		ImgGray img1;											//Declaring a grayscale image
		Load(lfilename, &img1b);									//Loading the BGR image
		Convert(img1b, &img1);										//Loading the BGR image into img1 as a grayscale image

		ImgFloat dmap;
		dmap.Reset(img1.Width(),img1.Height());
		Set(&dmap,0);

		ImgFloat dmaplrf;
		dmaplrf.Reset(img1.Width(),img1.Height());
		Set(&dmaplrf,0);

		ImgFloat dmaplr;
		dmaplr.Reset(img1.Width(),img1.Height());
		Set(&dmaplr,0);

		ImgFloat dmaplr1;
		dmaplr1.Reset(img1.Width(),img1.Height());
		Set(&dmaplr1,0);

		ImgFloat dmaplr2;
		dmaplr1.Reset(img1.Width(),img1.Height());
		Set(&dmaplr1,0);

		ImgBgr img2b; 											//Declaring BGR image
		ImgGray img2b;											//Declaring a grayscale image
		Load(rfilename, &img2b);									//Loading the BGR image
		Convert(img2b, &img2);										//Loading the BGR image into img1 as a grayscale image

		Figure fig1("Left image BGR");									//Displaying original image 
		fig1.Draw(img1b);							 
		Figure fig2("Right image BGR");									//Displaying BGR image
		fig2.Draw(img2b);
		Figure fig3("Left image");									//Displaying original image 
		fig3.Draw(img1);		 
		Figure fig4("Right image");									//Displaying BGR image
		fig4.Draw(img2);

		
		ImgFloat tmp;											//Intermediate image in convolution
		tmp.Reset(img1.Width(),img1.Height());
		Set(&tmp,0);

		ImgFloat cnvout;										//Intermediate image in convolution
		cnvout.Reset(img1.Width(),img1.Height());
		Set(&cnvout,0);

		ImgFloat depth;											//Intermediate image in convolution
		depth.Reset(img1.Width(),img1.Height());
		Set(&depth,0);

		int k=1315;

		std::vector<ImgFloat> a(maxdisparity);								//Vector of images to be stored according to order of disparity
		ImgFloat ds;											//Intermediate image in convolution
		ds.Reset(img1.Width(),img1.Height());
		Set(&cnvout,0);

		for(int d=0;d<maxdisparity;d++){
			a[d].Reset(img1.Width(),img1.Height());
			Set(&a[d],0);
			for(int y=0;y<img1.Height()-1;y++){
				for(int x=0;x<img1.Width()-1;x++){
					if((x-d)>=0){
						a[d](x,y)=abs(img1(x,y)-img2(x-d,y));
					}
				}
			}
		}
		float val;
		int x;
		int y;
		int i;

		int ar[7]={1,1,1,1,1,1,1};

		for(int d=0;d<maxdisparity;d++){								//computing disparity map
			for(y=0;y<img1.Height();y++){
				for(x=3;x<(img1.Width()-3);x++){
					val=0;
					for(i=0;i<7;i++){
						val=val+ar[i]*a[d](x+3-i,y);
					}
					tmp(x,y)=val;					
				}
			}
			for(y=3;y<(img1.Height()-3);y++){
				for(x=0;x<img1.Width();x++){
					val=0;
					for(i=0;i<7;i++){
						val=val+ar[i]*tmp(x,y+3-i);
					}
				    a[d](x,y)=val;
				}
			}			
		} 	
		
		for(y=0;y<img1.Height();y++){
				for(x=0;x<img1.Width();x++){
					int gc=999999;
					for(int d=0;d<maxdisparity;d++){
						if(a[d](x,y)<gc){
							gc=a[d](x,y);
							dmap(x,y)=d;
						}
					}
				}
		}
		Figure fig5("Disparity map");									//Displaying disparity map image
		fig5.Draw(dmap);

		for(y=0;y<img1.Height();y++){							
				for(x=0;x<img1.Width();x++){
					int gc=999999;
					for(int d=0;d<maxdisparity;d++){
						if(a[d](x,y)<gc){
							gc=a[d](x,y);
							dmaplr(x,y)=d;
						}
					}
				}
		}

		for(y=0;y<img1.Height();y++){
			for(x=0;x<img1.Width();x++){
				int gc=999999;
				for(int d=0;d<maxdisparity;d++){
					if((x-dmaplr(x,y)+d)>0 && (x-dmaplr(x,y)+d)<img1.Width()){
						if(a[d](x-dmaplr(x,y)+d,y)<gc){
							gc=a[d](x-dmaplr(x,y)+d,y);
							dmaplr1(x,y)=d;
						}							
					}
				}
			}
		}

		for(y=0;y<img1.Height();y++){
			for(x=0;x<img1.Width();x++){
				if(dmaplr(x,y)==dmaplr1(x,y)){
					dmaplrf(x,y)=dmaplr(x,y);
				}
				else{
					dmaplrf(x,y)=0;
				}
			}
		}

		Figure fig6("Disparity map with left right consistency check");					//Displaying disparity map with left right consistency check
		fig6.Draw(dmaplrf);

		for(y=0;y<img1.Height();y++){
			for(x=0;x<img1.Width();x++){
				if(dmaplrf(x,y)!=0){
					depth(x,y)=k/dmaplrf(x,y);
				}
			}
		}

		Figure fig7("Depth image");									//Displaying depth image
		fig7.Draw(depth);

		int vertices=0;

		for(y=0;y<img1.Height();y++){
			for(x=0;x<img1.Width();x++){
				if(dmaplrf(x,y)!=0){
					vertices++;
				}
			}
		}
		cout<<endl<<"Vertices="<<vertices<<endl;

		//Writing out 3-D image into .ply format that can be viewed in Meshlab
		ofstream myply; 
		myply.open("myply.ply");
		myply<<"ply"<<endl<<"format ascii 1.0"<<endl<<"element vertex "<<vertices<<endl<<"property float x"<<endl<<"property float y"<<endl<<"property float z"<<endl<<"property uchar diffuse_red"<<endl<<"property uchar diffuse_green"<<endl<<"property uchar diffuse_blue"<<endl<<"end_header"<<endl;

		for(y=0;y<img1.Height();y++){
			for(x=0;x<img1.Width();x++){
				if(depth(x,y)>0){
					myply<<x<<" "<<-y<<" "<<-depth(x,y)<<" "<<(int)img1b(x,y).r<<" "<<(int)img1b(x,y).g<<" "<<(int)img1b(x,y).b<<endl;
				}
			}
		}
		EventLoop();
	}
	

	catch (const Exception& e)
	{
		e.Display();    										// display exception to user in a popup window 
	}
	return 0;
}
