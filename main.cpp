#include <cstdlib>
#include <iostream>
#include <math.h>
#include <map>
#include <vector>
#include <set>
#include "image.h"

// ===================================================================================================
//Jing Ru Li
//Perfect Hashing
// ===================================================================================================
int find_pixels(const Image<Color> &img, Image<bool> & occ){ 
//finds the number of non-white pixels in the input, and sets up the occupancy file
    int p = 0;
    for(int i = 0; i < img.Width(); i++){
        for(int j = 0 ;j < img.Height(); j++){
            if(img.GetPixel(i, j).isWhite() == false){
                p++;         
                occ.SetPixel(i, j, true);
            }
            else
                occ.SetPixel(i, j, false);
        }
    }
    return p;
}
int gcd(int a, int b){ 
//use the euclidean algorithm to find gcd
    if(b == 0)
        return a;
    else
        return gcd(b, a%b);
}
void no_factors(int &n1, int &n2){ 
//increases offset and hash size until they don't have a common denominator higher than 1
    int i = 0;
    while(gcd(n1, n2) > 1){
        if(i%2 == 0)
           n1++;
        else
           n2++;
        i++;
    }
}
bool check_collision(const std::map<std::pair<int, int>, std::vector<std::pair<int, int> > >::iterator &high_track, const Image<Color> &img, Image<Color> &hash, Image<Offset> &offset, const std::set<std::pair<int, int> > &avail){
    for(std::set<std::pair<int, int> >::iterator itr = avail.begin(); itr != avail.end(); itr++){ 
	//avail contains all possible dx/dy combinations not already used
        for(int i = 0; i < high_track->second.size(); i++){ 
		//this will go through all the pixels in the specific offset cell
            if(hash.GetPixel((high_track->second[i].first+itr->first)%hash.Width(), (high_track->second[i].second+itr->second)%hash.Height()).isWhite() == false){
                //the above if statement checks if the cell it's going to be in is already filled or not, if so then I backtrack and erase all the pixels I wrote to that
                //contained that dx/dy offset combination
   
                Color white;
                for(int j = 0; j < i; j++){
                    hash.SetPixel((high_track->second[j].first+itr->first)%hash.Width(), (high_track->second[j].second+itr->second)%hash.Height(), white);
                }
                break;
            }
            else{
                //if the cell is empty, then hash and offset are written to using the same arithmetic as uncompress, with offset storing the dx/dy pair
                hash.SetPixel((high_track->second[i].first+itr->first)%hash.Width(), (high_track->second[i].second+itr->second)%hash.Height(), img.GetPixel(high_track->second[i].first, high_track->second[i].second));
                Offset offsets(itr->first, itr->second);
                offset.SetPixel((high_track->second[i].first)%offset.Width(), (high_track->second[i].second)%offset.Height(), offsets);
                if(i == high_track->second.size()-1){
                    //if it's reached the end of the vector then it means all cells were free of collision and I erase the dx/dy combination so I don't reuse it again
                    
                    return false;
                }
            }
        }
    }
    //if it never reached that point it means that collision free cells were not available and we'll probably have to expand offset and/or hashdata
    return true;
}
bool make_table(const Image<Color> &input, Image<Color> &hash, Image<Offset> &offset){
    //the following map will store the offset values as the key and the actual coordinates as values stored in a vector so we can later check which vector to start with (the largest)
    //the map is very similar to the offset cells pictured in the assignment
    std::map<std::pair<int, int>, std::vector<std::pair<int, int> > > off;
    std::pair<int, int> tmp;
    for(int i = 0; i < input.Width(); i++){
        for(int j =0; j < input.Height(); j++){
            if(input.GetPixel(i, j).isWhite() == false){
                tmp = std::make_pair(i%offset.Width(), j%offset.Height());
                off[tmp].push_back(std::make_pair(i, j));
     
            }
            
        }
    }

    std::set<std::pair<int, int> > available; 
	//possible values for dx/dy are 0-15, all of which are being stored into a set
    for(int i = 0; i < 16; i++){
        for(int j = 0; j < 16; j++){
            available.insert(std::make_pair(i, j));
        }
    }
 
    while(off.size() != 0){ 
	//runs until all of the offset cells have been allocated
        int high = 0;
        std::map<std::pair<int, int>, std::vector<std::pair<int, int> > >::iterator high_track; //this will point to the vector in the map with the most pixels in the offset cells
        for(std::map<std::pair<int, int>, std::vector<std::pair<int, int> > >::iterator itr = off.begin(); itr != off.end(); itr++){ //looks for the biggest vector
            if(itr->second.size() > high){
                high = itr->second.size();
                high_track = itr;
            }
        }
        
        if(check_collision(high_track, input, hash, offset, available) == false) //allocates the pixels, if no collisions were detected, the function returns false
            off.erase(high_track); //the portion of the map is erased since it's been allocated
        else{ //if collisions were inevitable then false is returned where the offset and/or hashtables will increase in size
\
            return false;
        }
     
    }
    return true;
}
void Compress(const Image<Color> &input, 
              Image<bool> &occupancy, Image<Color> &hash_data, Image<Offset> &offset) {
    //sets up occupancy in the first two lines
    occupancy.Allocate(input.Width(), input.Height());
    int p = find_pixels(input, occupancy);
    //using the recommended expressions for size
    int hashsize = ceil(sqrt(1.01*p));
    int offsize = ceil(sqrt(p/4));
    //the following function is called to resize hashsize and offsize so they don't share a gcd higher than 1
    no_factors(hashsize, offsize);
    //the hashsize and offsize tables are allocated
    Color white;
    offset.Allocate(offsize, offsize);
    hash_data.Allocate(hashsize, hashsize);
    hash_data.SetAllPixels(white);
    int i = 0;
    //this function resizes hash_data and offsize if not all the pixels could be fit uniquely into the tables, going back and forth in increasing
    while(make_table(input, hash_data, offset) != true){
        if(i%2 == 0){
            //since the previous tables couldn't support the pixels, they're reset to start anew with a bigger size
            //I increment the size of each one every other time so as to not waste space unnecessarily
            offsize++;
            offset.Allocate(offsize, offsize);
            hash_data.Allocate(hashsize, hashsize);
            hash_data.SetAllPixels(white);
            
        }
        else{
            hashsize++;
            hash_data.Allocate(hashsize, hashsize);
            hash_data.SetAllPixels(white);
            offset.Allocate(offsize, offsize);

        }
        i++;
  
    }
  
    
    

}


void UnCompress(const Image<bool> &occupancy, const Image<Color> &hash_data, const Image<Offset> &offset, 
                Image<Color> &output) {

    int dx, dy; 
   //initialize the space required for the output
    output.Allocate(occupancy.Width(), occupancy.Height());
    Color white;
    //set everything to white as default
    output.SetAllPixels(white);
    for(int i = 0; i < occupancy.Width(); i++){
        for(int j =0; j < occupancy.Height(); j++){
            if(occupancy.GetPixel(i, j) == true){
                //if a pixel exists then I find the offset value and use it to obtain the pixel, and this runs for the entire image
                dx = offset.GetPixel(i%offset.Width(), j%offset.Height()).dx;
                dy = offset.GetPixel(i%offset.Width(), j%offset.Height()).dy;
                output.SetPixel(i, j, hash_data.GetPixel((i+dx)%hash_data.Width(), (j+dy)%hash_data.Height()));
            }
        }
    }

   

}


// ===================================================================================================
// ===================================================================================================

void Compare(const Image<Color> &input1, const Image<Color> &input2, Image<bool> &output) {

  // confirm that the files are the same size
  if (input1.Width() != input2.Width() ||
      input1.Height() != input2.Height()) {
    std::cerr << "Error: can't compare images of different dimensions: " 
         << input1.Width() << "x" << input1.Height() << " vs " 
         << input2.Width() << "x" << input2.Height() << std::endl;
  } else {
    
    // confirm that the files are the same size
    output.Allocate(input1.Width(),input1.Height());
    int count = 0;
    for (int i = 0; i < input1.Width(); i++) {
      for (int j = 0; j < input1.Height(); j++) {
        Color c1 = input1.GetPixel(i,j);
        Color c2 = input2.GetPixel(i,j);
        if (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b)
          output.SetPixel(i,j,true);
        else {
          count++;
          output.SetPixel(i,j,false);
        }
      }      
    }     

    // confirm that the files are the same size
    if (count == 0) {
      std::cout << "The images are identical." << std::endl;
    } else {
      std::cout << "The images differ at " << count << " pixel(s)." << std::endl;
    }
  }
}

// ===================================================================================================
// ===================================================================================================


// to allow visualization of the custom offset image format
void ConvertOffsetToColor(const Image<Offset> &input, Image<Color> &output) {
  // prepare the output image to be the same size as the input image
  output.Allocate(input.Width(),input.Height());
  for (int i = 0; i < output.Width(); i++) {
    for (int j = 0; j < output.Height(); j++) {
      // grab the offset value for this pixel in the image
      Offset off = input.GetPixel(i,j);
      // set the pixel in the output image
      int dx = off.dx;
      int dy = off.dy;
      assert (dx >= 0 && dx <= 15);
      assert (dy >= 0 && dy <= 15);
      // to make a pretty image with purple, cyan, blue, & white pixels:
      int red = dx * 16;
      int green = dy *= 16;
      int blue = 255;
      assert (red >= 0 && red <= 255);
      assert (green >= 0 && green <= 255);
      output.SetPixel(i,j,Color(red,green,blue));
    }
  }
}


// ===================================================================================================
// ===================================================================================================

void usage(char* executable) {
  std::cerr << "Usage:  4 options" << std::endl;
  std::cerr << "  1)  " << executable << " compress input.ppm occupancy.pbm data.ppm offset.offset" << std::endl;
  std::cerr << "  2)  " << executable << " uncompress occupancy.pbm data.ppm offset.offset output.ppm" << std::endl;
  std::cerr << "  3)  " << executable << " compare input1.ppm input2.ppm output.pbm" << std::endl;
  std::cerr << "  4)  " << executable << " visualize_offset input.offset output.ppm" << std::endl;
}


// ===================================================================================================
// ===================================================================================================

int main(int argc, char* argv[]) {
  if (argc < 2) { usage(argv[1]); exit(1); }

  if (argv[1] == std::string("compress")) {
    if (argc != 6) { usage(argv[1]); exit(1); }
    // the original image:
    Image<Color> input;
    // 3 files form the compressed representation:
    Image<bool> occupancy;
    Image<Color> hash_data;
    Image<Offset> offset;
    input.Load(argv[2]);
    Compress(input,occupancy,hash_data,offset);
    // save the compressed representation
    occupancy.Save(argv[3]);
    hash_data.Save(argv[4]);
    offset.Save(argv[5]);

  } else if (argv[1] == std::string("uncompress")) {
    if (argc != 6) { usage(argv[1]); exit(1); }
    // the compressed representation:
    Image<bool> occupancy;
    Image<Color> hash_data;
    Image<Offset> offset;
    occupancy.Load(argv[2]);
    hash_data.Load(argv[3]);
    offset.Load(argv[4]);
    // the reconstructed image
    Image<Color> output;
    UnCompress(occupancy,hash_data,offset,output);
    // save the reconstruction
    output.Save(argv[5]);
  
  } else if (argv[1] == std::string("compare")) {
    if (argc != 5) { usage(argv[1]); exit(1); }
    // the original images
    Image<Color> input1;
    Image<Color> input2;    
    input1.Load(argv[2]);
    input2.Load(argv[3]);
    // the difference image
    Image<bool> output;
    Compare(input1,input2,output);
    // save the difference
    output.Save(argv[4]);

  } else if (argv[1] == std::string("visualize_offset")) {
    if (argc != 4) { usage(argv[1]); exit(1); }
    // the 8-bit offset image (custom format)
    Image<Offset> input;
    input.Load(argv[2]);
    // a 24-bit color version of the image
    Image<Color> output;
    ConvertOffsetToColor(input,output);
    output.Save(argv[3]);

  } else {
    usage(argv[0]);
    exit(1);
  }
}
