#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define SQR(x) ((x)*(x))
void main()

{	
	FILE		*infile;
	FILE		*outfile;
	FILE		*fpt;
	unsigned char	*image;
	unsigned char	*ac;
	int		*gx;
	int		*gy;
	float		*eint1;
	float		*eint2;
	float		*eext;
	float		*energy;	
	unsigned char	*grad;
	char		ch[42];
	int		mnc;	
	int		mnr;
	int 		x[42][2];
	int		min;
	int		max;	
	char		header[320];
	int		ROWS,COLS,BYTES;
	int		r,c,r2,c2,sum;
	int		temp_sum=0;
	int		sobel_x[9]={-1, 0, 1, -2, 0, 2, -1, 0, 1};
	int		sobel_y[9]={-1, -2, -1, 0, 0, 0, 1, 2, 1};

	/* read image */
	if ((fpt=fopen("hawk.ppm","rb")) == NULL)
	{
		printf("Unable to open [hawk.ppm for reading\n");
		exit(0);
	}
	fscanf(fpt,"%s %d %d %d\n",header,&COLS,&ROWS,&BYTES);
	if (strcmp(header,"P5") != 0  ||  BYTES != 255)
	{
		printf("Not a greyscale 8-bit PPM image\n");
		exit(0);
	}
	image=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));

	fread(image,1,COLS*ROWS,fpt);
	fclose(fpt);

	/* allocate memory for active contour image */
	ac=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
	for(r=0;r<ROWS;r++)
	{
		for(c=0;c<COLS;c++)
		{
			ac[r*COLS+c]=255;
		}
	}	

	/* allocate memory for Sobel x image */
	gx=(int *)calloc(ROWS*COLS,sizeof(int));

	/* convolve image, skipping the border points for gx */
	for (r=1; r<ROWS-1; r++)//Scanning image rows, neglecting first 3 pixels
	{
		for (c=1; c<COLS-1; c++)//Scanning image columns, neglecting first 3 pixels
		{
			sum=0;
			for (r2=-1; r2<=1; r2++)//Iterator through kernel rows
			{
				for (c2=-1; c2<=1; c2++)//Iterator through kernel columns
					{
						sum+=image[(r+r2)*COLS+(c+c2)]*sobel_x[(r2+1)*3+(c2+1)];			
					}
			}
			gx[r*COLS+c]=sum;
		}
	}

	/* allocate memory for Sobel y image */
	gy=(int *)calloc(ROWS*COLS,sizeof(int));

	/* convolve gx, skipping the border points for gy */
	for (r=1; r<ROWS-1; r++)//Scanning image rows, neglecting first 3 pixels
	{
		for (c=1; c<COLS-1; c++)//Scanning image columns, neglecting first 3 pixels
		{
			sum=0;
			for (r2=-1; r2<=1; r2++)//Iterator through kernel rows
			{
				for (c2=-1; c2<=1; c2++)//Iterator through kernel columns
					{
						sum+=image[(r+r2)*COLS+(c+c2)]*sobel_y[(r2+1)*3+(c2+1)];			
					}
			}
			gy[r*COLS+c]=sum;
		}
	}

	/* allocate memory for Sobel y image */
	grad=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));

	/* compute magitude */
	for (r=1; r<ROWS-1; r++)//Scanning image rows, neglecting first 3 pixels
	{
		for (c=1; c<COLS-1; c++)//Scanning image columns, neglecting first 3 pixels
		{
			grad[r*COLS+c]=abs(gx[r*COLS+c])+abs(gy[r*COLS+c]);
			if(grad[r*COLS+c]>255)
				grad[r*COLS+c]=255;
			if(grad[r*COLS+c]<0)
				grad[r*COLS+c]=0;
		}
	}
 
	/* allocate memory for internal energy 1 */
	eint1=(float *)calloc(ROWS*COLS,sizeof(float));

	/* allocate memory for internal energy 2 */
	eint2=(float *)calloc(ROWS*COLS,sizeof(float));

	/* allocate memory for external energy */
	eext=(float *)calloc(ROWS*COLS,sizeof(float));

	/* allocate memory for external energy */
	energy=(float *)calloc(ROWS*COLS,sizeof(float));

	/* creating 2D array of the contour pixels*/
	infile = fopen("hawk_init.txt", "r");       // using relative path name of file
	if (infile == NULL) 
	{  
		// printing out the passed error string and calling exit(1);
		printf("Unable to open file."); 
		exit(0);
	}
	int i;	
	int j;
	for(i=0;i<42;i++)
	{
		fscanf(infile, "%d %d\n", &x[i][0], &x[i][1]);
	}
	fclose(infile);
	
	outfile = fopen("output.txt","w");
	for(i=0;i<42;i++)
	{
		fprintf(outfile, "%d %d\n", x[i][0], x[i][1]);
	}
	fclose(outfile);
	float	*e1;
	float	*e2;
	float	*e3;

	/* allocate memory for internal energy 1, normalised */
	e1=(float *)calloc(ROWS*COLS,sizeof(float));

	/* allocate memory for internal energy 2, normalised */
	e2=(float *)calloc(ROWS*COLS,sizeof(float));

	/* allocate memory for external energy, normalised */
	e3=(float *)calloc(ROWS*COLS,sizeof(float));
	float	mn1;
	float  	mn2;
	float	mn3;
	float	mx1;
	float  	mx2;
	float	mx3;
	float	rng;
	int iter;
	int l,m;
	for(iter=0;iter<100;iter++)
	{	
		sum=0;
		int avg=0;
		for(i=0;i<42;i++)
		{
			if(i!=41)
				sum+=(SQR(x[i][0]-x[i+1][0])+SQR(x[i][1]-x[i+1][1]));
			else
				sum+=(SQR(x[i][0]-x[0][0])+SQR(x[i][1]-x[0][1]));
		}
		avg=sum/42;
		//printf("%d\t",avg);
		for(i=0;i<42;i++)
		{
                        if(i==0)
                        {
				l=x[0][0];
				m=x[0][1];
			}
			mnc=x[i][0];
			mnr=x[i][1];
			for(r2=x[i][1]-3;r2<=x[i][1]+3;r2++)
			{
				for(c2=x[i][0]-3;c2<=x[i][0]+3;c2++)
				{
					if(i!=41)
					{
						eint1[r2*COLS+c2]=SQR(x[i+1][0]-c2)+SQR(x[i+1][1]-r2);
					}
					else
					{
						eint1[r2*COLS+c2]=SQR(l-c2)+SQR(m-r2);
					}
					eint2[r2*COLS+c2]=abs(avg-eint1[r2*COLS+c2]);
					eext[r2*COLS+c2]=-(float)grad[r2*COLS+c2];
				}
                        }

			mn1=eint1[(x[i][1]-3)*COLS+(x[i][0]-3)];
			mx1=eint1[(x[i][1]-3)*COLS+(x[i][0]-3)];
			/* computing the normalised internal energy 1 */
			for(r2=x[i][1]-3;r2<=x[i][1]+3;r2++)
			{
				for(c2=x[i][0]-3;c2<=x[i][0]+3;c2++)
				{
					if(mn1>=eint1[r2*COLS+c2])
					{
						mn1=eint1[r2*COLS+c2];
					}
					else if(mx1<eint1[r2*COLS+c2])
					{
						mx1=eint1[r2*COLS+c2];
					}
				}
			}
							
			//printf("%f\t",mn1);		
			//printf("%f\t",mx1);
			rng=mx1-mn1;
			for(r2=x[i][1]-3;r2<=x[i][1]+3;r2++)
			{
				for(c2=x[i][0]-3;c2<=x[i][0]+3;c2++)
				{
					e1[r*COLS+c]=(eint1[r*COLS+c]-mn1)/rng;
					//printf("%f\t",e1[r2*COLS+c2]);
				}
			}
			/* computing the normalised internal energy 2 */			
			mn2=eint2[(x[i][1]-3)*COLS+(x[i][0]-3)];
			mx2=eint2[(x[i][1]-3)*COLS+(x[i][0]-3)];
			for(r2=x[i][1]-3;r2<=x[i][1]+3;r2++)
			{
				for(c2=x[i][0]-3;c2<=x[i][0]+3;c2++)
				{
					if(mn2>=eint2[r2*COLS+c2])
					{
						mn2=eint2[r2*COLS+c2];
					}
					if(mx2<eint2[r2*COLS+c2])
					{
						mx2=eint2[r2*COLS+c2];
					}
				}
			}		
			//printf("%f\t",mn2);		
			//printf("%f\t",mx2);
			rng=mx2-mn2;
			for(r2=x[i][1]-3;r2<=x[i][1]+3;r2++)
			{
				for(c2=x[i][0]-3;c2<=x[i][0]+3;c2++)
				{
					e2[r*COLS+c]=(eint2[r*COLS+c]-mn2)/rng;
					//printf("%f\t",e2[r2*COLS+c2]);
				}
			}
			/* computing the normalised external energy */
			mn3=eext[(x[i][1]-3)*COLS+(x[i][0]-3)];
			mx3=eext[(x[i][1]-3)*COLS+(x[i][0]-3)];
			for(r2=x[i][1]-3;r2<=x[i][1]+3;r2++)
			{
				for(c2=x[i][0]-3;c2<=x[i][0]+3;c2++)
				{
					if(mn3>=eext[r2*COLS+c2])
					{
						mn3=eext[r2*COLS+c2];
					}
					if(mx3<eext[r2*COLS+c2])
					{
						mx3=eext[r2*COLS+c2];
					}
				}
			}		
			//printf("%f\t",mn3);		
			//printf("%f\t",mx3);		
			rng=mx3-mn3;
			for(r2=x[i][1]-3;r2<=x[i][1]+3;r2++)
			{
				for(c2=x[i][0]-3;c2<=x[i][0]+3;c2++)
				{
					e3[r*COLS+c]=(eext[r*COLS+c]-mn3)/rng;
					//printf("%f\t",e3[r2*COLS+c2]);
				}
			}
			/* computing total energy */		
			for(r2=x[i][1]-3;r2<=x[i][1]+3;r2++)
			{
				for(c2=x[i][0]-3;c2<=x[i][0]+3;c2++)
				{			
					//energy[r2*COLS+c2]=e1[r2*COLS+c2]+e2[r2*COLS+c2]+e3[r2*COLS+c2];		
					energy[r2*COLS+c2]=eint1[r2*COLS+c2]+eint2[r2*COLS+c2]+eext[r2*COLS+c2];
					//printf("%f\t",energy[r2*COLS+c2]);
				}
			}
			/* finding pixel location for minimum energy */			
			min=energy[(x[i][1]-3)*COLS+(x[i][0]-3)];
			for(r2=x[i][1]-3;r2<=x[i][1]+3;r2++)
			{
				for(c2=x[i][0]-3;c2<=x[i][0]+3;c2++)
				{
					if(min>energy[r2*COLS+c2])
					{
						mnc=c2;
						mnr=r2;
						min=energy[r2*COLS+c2];
					}
				}
			}
			x[i][1]=mnr;
			x[i][0]=mnc;
		}           
	        /*Writing ac image*/
	        fpt=fopen("ac.ppm","w");
	        if (fpt == NULL)
	        {
		        printf("Unable to open hawk.ppm for writing\n");
		        exit(0);
	        }
	        fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
	        fwrite(ac,1,ROWS*COLS,fpt);
	        fclose(fpt);
	}
	
		
	/*listing out the final contour points */
	outfile = fopen("outputf.txt","w");
	for(i=0;i<42;i++)
	{
		fprintf(outfile, "%d %d\n", x[i][0], x[i][1]);
	}
	fclose(outfile);

	/* marking the contour image */
	for(i=0;i<42;i++)
	{
		c=x[i][0];
		for(r=x[i][1]-3;r<=x[i][1]+3;r++)
		{
			ac[r*COLS+c]=0;
		}
		r=x[i][1];
		for(c=x[i][0]-3;c<=x[i][0]+3;c++)
		{
			ac[r*COLS+c]=0;
		}
	}
	for(r=0;r<ROWS;r++)
	{
		for(c=0;c<COLS;c++)
		{
			if(ac[r*COLS+c]!=0)
			{
				ac[r*COLS+c]=image[r*COLS+c];
			}
		}
	}
	
	/*Writing original image*/
	fpt=fopen("Original.ppm","w");
	if (fpt == NULL)
	{
		printf("Unable to open hawk.ppm for writing\n");
		exit(0);
	}
	fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
	fwrite(image,1,ROWS*COLS,fpt);
	fclose(fpt);

	/*Writing ac image*/
	fpt=fopen("ac100.ppm","w");
	if (fpt == NULL)
	{
		printf("Unable to open hawk.ppm for writing\n");
		exit(0);
	}
	fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
	fwrite(ac,1,ROWS*COLS,fpt);
	fclose(fpt);

	/*Writing sobel image*/
	fpt=fopen("sobel.ppm","w");
	if (fpt == NULL)
	{
		printf("Unable to open hawk.ppm for writing\n");
		exit(0);
	}
	fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
	fwrite(grad,1,ROWS*COLS,fpt);
	fclose(fpt);	
}
