// Author: Nicholas Heim
// Project: Seam Carving
// uakron email: nwh8
// Last Edited: Fri, Dec 13, 2019 23:42:36

#include <cstdlib>
#include <array>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <sstream>
#include <algorithm>


// Using statements
using std::cout;
using std::endl;
using std::fstream;
using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;
using std::abs;

class Image
{
private:
   unsigned char **matrix;
   unsigned short **energy;
   int rows, columns, **sums;
   int irows; // Because of how the value of rows is adjusted, this value is needed to ensure 
              // proper deletion of the original matrix and energy arrays.
   unsigned char colors;
   vector<string> errors{"File failed to open or was not found.",
                         "File does not adhere to specified format.",
                         "Row or column count is not greater than zero.",
                         "Color count is greater than 255, overflow error."};
   void energyCalc();
   void leftShift(int r, int start);
   void upShift(int c, int start);

   // Helper functions
   int min(int, int, int);
   int min2(int, int);

public:
   // Constructors
   Image(string filename);

   // Functions for seam carving. They will be similar, but not identical.
   void verCarve();
   void horCarve();

   // Saving Function
   void save(string filename);

   // Destructor
   ~Image();
};



int main(int argc, char const *argv[])
{
   string filename = argv[1];
   Image file(filename);

   int times = std::stoi(argv[2], nullptr, 10);
   for (int i = 0; i < times; ++i)
      file.verCarve();
   
   times = std::stoi(argv[3], nullptr, 10);
   for(int i = 0; i < times; ++i)
      file.horCarve();
   
   file.save(filename);
   
   return 0;
}

/* Desc: This function is the main constructor. It gathers the required information from the file.
 *       It should be noted that the input file is expected to have the following form in the first
 *       four lines: 
 *       P2
 *       # Created by IrfanView
 *       ROWS COLUMNS
 *       COLOR_SHADE_COUNT  //Presumed to be 255, grayscale/monochromatic. 
 *  Pre: Requires that the name of a file to be opened is passed through the console.
 * Post: The container is initialized with the data within the file. 
 */
Image::Image(string filename)
{
   try
   {
      ifstream input(filename, std::ios::in);
      if(!input.is_open())
         throw errors.at(0);

      string temp;
      // Discard first two lines, assuming that the second is 
      // a comment, ignored otherwise
      getline(input, temp);
      if(input.peek() == '#')
         getline(input, temp);
      temp.clear();

      // Read the file as specified above.
      getline(input, temp, ' ');
      columns = std::stoi(temp, nullptr, 10);
      getline(input, temp);
      rows = std::stoi(temp, nullptr, 10);
      irows = rows;

      if(rows <= 0 || columns <= 0)
         throw errors.at(2);

      getline(input, temp);
      colors = (unsigned char)(std::stoi(temp, nullptr, 10));
      if(colors > 255 || colors < 0)
         throw errors.at(3);

      matrix = new unsigned char*[rows];
      for (size_t i = 0; i < rows; ++i)
         matrix[i] = new unsigned char[columns];
      
      energy = new unsigned short*[rows];
      for (size_t i = 0; i < rows; ++i)
         energy[i] = new unsigned short[columns];

      sums = new int*[rows];
      for (size_t i = 0; i < rows; ++i)
         sums[i] = new int[columns];

      string whitespace = " \t\n";
      int i = 0, j = 0, position = 0;
      // IMPORTANT: An assumption is made that EVERY number in the file passed has a space or tab after it. 
      // This will come into play when we are bounds checking the string. It is also assumed that the 
      // beginning of each line has no whitespace. 
      while(i != rows)
      {
         getline(input, temp);
         do
         {
            matrix[i][j] = std::stoi(temp.substr(position, temp.find_first_of(whitespace, position + 1) - position));
            position = temp.find_first_of(whitespace, position + 1);

            ++j;
            if(j == columns)
            {
               j = 0;
               ++i;
            }
         } while (temp.find_first_of(whitespace, position + 1) <= temp.find_last_of(whitespace));
         
         if(j == columns - 1)
         {  
            matrix[i][j] = std::stoi(temp.substr(position, temp.find_first_of(whitespace, position + 1) - position));
            j = 0;
            ++i;
         }
         position = 0;  
      }

      input.close();
   }
   catch(const std::exception& e)
   {
      std::cerr << e.what() << '\n';
   }
}

/* Desc: Calculates the energy value of each matrix. This value (at i, j) is found by finding the difference 
 *       between the value at matrix[i][j] and adjacent values, given they exist (not including diagonal values).
 *  Pre: Requires that the matrix of values is calculated first.
 * Post: The energy matrix is now calculated.
 */
void Image::energyCalc()
{   
   for (size_t i = 0; i < rows; ++i)
   {
      for (size_t j = 0; j < columns; ++j)
      {
         if(i != 0 && i != rows - 1)
         {
            if(j != 0 && j != columns - 1)
               energy[i][j] = (unsigned short)(abs(matrix[i][j] - matrix[i - 1][j]) + abs(matrix[i][j] - matrix[i][j + 1]) +
                                               abs(matrix[i][j] - matrix[i + 1][j]) + abs(matrix[i][j] - matrix[i][j - 1]));
            else if (j == 0)
               energy[i][j] = (unsigned short)(abs(matrix[i][j] - matrix[i - 1][j]) + abs(matrix[i][j] - matrix[i][j + 1]) + abs(matrix[i][j] - matrix[i + 1][j]));
            else if (j == columns - 1)
               energy[i][j] = (unsigned short)(abs(matrix[i][j] - matrix[i - 1][j]) + abs(matrix[i][j] - matrix[i][j - 1]) + abs(matrix[i][j] - matrix[i + 1][j]));
         }
         else if(i == 0)
         {
            if(j != 0 && j != columns - 1)
               energy[i][j] = (unsigned short)(abs(matrix[i][j] - matrix[i][j - 1]) + abs(matrix[i][j] - matrix[i][j + 1]) + abs(matrix[i][j] - matrix[i + 1][j]));
            else if(j == 0)
               energy[i][j] = (unsigned short)(abs(matrix[i][j] - matrix[i][j + 1]) + abs(matrix[i][j] - matrix[i + 1][j]));
            else if (j == columns - 1)
               energy[i][j] = (unsigned short)(abs(matrix[i][j] - matrix[i][j - 1]) + abs(matrix[i][j] - matrix[i + 1][j]));
         }
         else if(i == rows - 1)
         {
            if(0 < j && j < columns - 1)
               energy[i][j] = (unsigned short)(abs(matrix[i][j] - matrix[i][j - 1]) + abs(matrix[i][j] - matrix[i][j + 1]) + abs(matrix[i][j] - matrix[i - 1][j]));
            else if(j == 0)
               energy[i][j] = (unsigned short)(abs(matrix[i][j] - matrix[i][j + 1]) + abs(matrix[i][j] - matrix[i - 1][j]));
            else if (j == columns - 1)
               energy[i][j] = (unsigned short)(abs(matrix[i][j] - matrix[i][j - 1]) + abs(matrix[i][j] - matrix[i - 1][j]));
         }
      }  
   }
}

/* 
 * Desc: Finds the minimum vertical seam. Removes the values of this seam, removes one from the 
 *       column count. Each call only removes one seam. 
 *  Pre: Requires that an image is loaded into the container.
 * Post: Removes the vertical seam of lowest energy.
 */
void Image::verCarve()
{
   // Calculate the enery matrix now.
   energyCalc();

   // Calculate the minimum seam in the sums array. This is done top down.
   // It takes the already calculated sum of the three pixels above and picks the smallest one.
   // This is then added with the energy at the current position to give the sum of the seam so far.
   // To save some comparisons, we do the top row separately.
   for (size_t i = 0; i < columns; ++i)
      sums[0][i] = energy[0][i];
   
   for (size_t i = 1; i < rows; ++i)
   {
      for (size_t j = 0; j < columns; ++j)
      {
         if(j != 0 && j != columns - 1)
            sums[i][j] = sums[i - 1][j + min(sums[i - 1][j - 1], sums[i - 1][j], sums[i - 1][j + 1])] + energy[i][j];
         else if (j == 0)
            sums[i][j] = std::min(sums[i - 1][j], sums[i - 1][j + 1]) + energy[i][j];
         else
            sums[i][j] = std::min(sums[i - 1][j - 1], sums[i - 1][j]) + energy[i][j];
      }
   }
   
   // Find the starting point of the minimum seam
   int start = 0;
   for (size_t i = 1; i < columns; ++i)
      if(sums[rows - 1][start] > sums[rows - 1][i])
         start = i;
   
   // Carve the seam out by writing over the value in the pixel matrix and working up the seam. 
   // NOTE: If two seam sums are the same, the LEFT one is removed. 
   for (size_t i = rows - 1; i > 0; --i)
   {
      if(start != 0 && start != columns - 1)
      {
         leftShift(i, start);
         start += min(sums[i - 1][start - 1], sums[i - 1][start], sums[i - 1][start + 1]);
      }
      else if(start == 0)
      {
         leftShift(i, start);
         start += min2(sums[i - 1][start], sums[i - 1][start + 1]) + 1;
      }
      else // start == columns - 1
      {
         leftShift(i, start);
         start += min2(sums[i - 1][start], sums[i - 1][start + 1]);
      }
   }
   leftShift(0, start);
   --columns;
}

/* 
 * Desc: Finds the minimum horizontal seam. Removes the values of this seam from the matrix.
 *       Decrements the row count. Each call removes one seam.
 *  Pre: Requires that an image is loaded into the container.
 * Post: Removes the horizontal seam of lowest energy.
 */
void Image::horCarve()
{
   // Calculate the enery matrix now.
   energyCalc();

   // Calculate the minimum seam in the sums array. This is done left to right.
   // It takes the already calculated sum of the three pixels to the left and picks the smallest one.
   // This is then added with the energy at the current position to give the sum of the seam so far.
   // To save some comparisons, we do the left column separately.
   for (size_t i = 0; i < rows; ++i)
      sums[i][0] = energy[i][0];
   
   for (size_t i = 1; i < columns; ++i)
   {
      for (size_t j = 0; j < rows; ++j)
      {
         if(j != 0 && j != rows - 1)
            sums[j][i] = sums[j + min(sums[j - 1][i - 1], sums[j][i - 1], sums[j + 1][i - 1])][i - 1] + energy[j][i];
         else if(j == 0)
            sums[j][i] = std::min(sums[j][i - 1], sums[j + 1][i - 1]) + energy[j][i];
         else // j == rows - 1
            sums[j][i] = std::min(sums[j - 1][i - 1], sums[j][i - 1]) + energy[j][i];
      }
   }

   // Find the starting row of the minimum seam
   int start = 0;
   for (size_t i = 1; i < rows; ++i)
      if(sums[start][columns - 1] > sums[i][columns - 1])
         start = i;
   
   // Carve the seam out by writing over the value in the pixel matrix and working across the seam.
   // NOTE: If two seam sums are the same, the TOP one is removed.
   for (size_t i = columns - 1; i > 0; --i)
   {
      if(start != 0 && start != rows - 1)
      {
         upShift(start, i);
         start += min(sums[start - 1][i - 1], sums[start][i - 1], sums[start + 1][i - 1]);
      }
      else if(start == 0)
      {
         upShift(start, i);
         start += min2(sums[start][i - 1], sums[start + 1][i - 1]) + 1;
      }
      else // i == rows - 1
      {
         upShift(start, i);
         start += min2(sums[start - 1][i - 1], sums[start][i - 1]);
      }
   }
   upShift(start, 0);
   --rows;
}

/* 
 * Desc: Saves the data from the cut image into a file by the specified name
 *  Pre: Requires that the constructor has run.
 * Post: The cut image is saved into a new .pgm file, does not overwrite.
 */
void Image::save(string filename)
{
   filename.insert(filename.find_last_of('.'), "_processed");
   ofstream output(filename, std::ios::out | std::ios::trunc);
   output << "P2" << endl
          << columns << " " << rows << " " << endl
          << "255" << endl;
   for (size_t i = 0; i < rows; ++i)
   {
      for (size_t j = 0; j < columns; ++j)
         output << (int)matrix[i][j] << " ";
      output << endl;
   }
   output.close();
}

/* 
 * Desc: Called by verCarve to move the remaining values of the row into the new position. 
 *  Pre: Requires that it is called with a row to adjust and a value in that row to start from.
 * Post: The row now has the value at the start position removed.
 */
void Image::leftShift(int r, int start)
{
   for (size_t i = start; i < columns - 1; ++i)
      matrix[r][i] = matrix[r][i + 1];
}

/* 
 * Desc: Called by horCarve to move the remaining values of the columns into the correct position.
 *  Pre: Requires that it is called with a column to adjust and a value to start from.
 * Post: The column has be shifted up one and the value at the initial position overwritten.
 */
void Image::upShift(int start, int c)
{
   for (size_t i = start; i < rows - 1; ++i)
      matrix[i][c] = matrix[i + 1][c];
}

// Basic destructor for Image.
Image::~Image()
{
   for (size_t i = 0; i < irows; ++i)
   {
      delete[] matrix[i];
      delete[] energy[i];
      delete[] sums[i];
   }

   delete[] matrix;
   delete[] energy;
   delete[] sums;

   // Always use protection!
   matrix = nullptr;
   energy = nullptr;
   sums = nullptr;
}

// Finds the positions of the minimum value, this will be used accordingly. 
// If it is called when the value is all the way to the left or right (or top / bottom)
// it will set the unused one to the max integer value to ensure it is not picked. 
// This must be accounted for by either adding 1 if on the left.
// The chance of this not working is basically 0, unless youre trying to break it. 
// Requires a picture almost than 2 million by 2 million pixels with a max energy value
// at each energy location. BW checkerboard essentially.
int Image::min(int x, int y, int z = 0x7fffffff)
{
   if(x <= y)
   {
      if(x <= z)
         return -1;
      else
         return 1;
   }
   else
   {
      if(y <= z)
         return 0;
      else
         return 1;
   }
}

int Image::min2(int x, int y)
{
   if(x <= y)
      return -1;
   return 0;
}

