Name	: 	Mriganka Haldavnekar
Email	: 	mhaldav@clemson.edu

Contents:	main.c					-	source code
			globals.h				-	global variable declaration
			interface.rc			-	GUI interface file
			resource.h				-	resource file for icon
			Region interaction.docx	-	project report

Description:	This C++ application was developed to perform region growing on the 
				user's desired ppm image. 
				Given an image, a starting point, and a label, this routine
				paint-fills (8-connected) the area with the given new label
				according to the following criteria(predicates) :
					1. absolute difference of the pixel intensity to the average 
					intensity of pixels alreadyin the region
					2. distance of the pixel to the centroid of the pixel already 
					in the region

Usage: 		Run executable file plus.exe at Region interaction > Region_interaction_files > Debug
			Use the GUI options to select mode, predicates and color.
			Check project report for option details.
			
Development IDE: Microsoft Visual Studio 2012
