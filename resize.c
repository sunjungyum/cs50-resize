// Sun-Jung Yum
// October 04 2018
// Problem Set 3

#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // Ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./resize n infile outfile\n");
        return 1;
    }

    // Read factor as a decimal and an integer to compare
    double possibledecimal = atof(argv[1]);
    int factor = atoi(argv[1]);

    // Ensure factor is an integer between 1 and 100
    if (factor < 1 || factor > 100 || possibledecimal != factor)
    {
        fprintf(stderr, "Can only resize with integer factors between 1 to 100\n");
        return 1;
    }

    // Remember filenames
    char *infile = argv[2];
    char *outfile = argv[3];

    // Open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // Open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // Read infile's BITMAPFILEHEADER and make a BITMAPFILEHEADER for resized version
    BITMAPFILEHEADER bf, newbf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);
    newbf = bf;

    // Read infile's BITMAPINFOHEADER and make a BITMAPINFOHEADER for resized version
    BITMAPINFOHEADER bi, newbi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);
    newbi = bi;

    // Ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }

    // Change dimensions of newbi based on factor
    newbi.biWidth *= factor;
    newbi.biHeight *= factor;

    // Determine original and new padding for scanlines
    int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    int newPadding = (4 - (newbi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // Change dimensions of new biSizeImage
    newbi.biSizeImage = abs(newbi.biHeight) * ((newbi.biWidth * sizeof(RGBTRIPLE)) + newPadding);

    // Change dimensions of bf based on factor
    newbf.bfSize = newbi.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // Write outfile's BITMAPFILEHEADER with new bf
    fwrite(&newbf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // Write outfile's BITMAPINFOHEADER with new bi, after being multiplied by factor
    fwrite(&newbi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // Iterate over infile's scanlines
    for (int i = 0; i < abs(bi.biHeight); i++)
    {
        // Write scanline to outfield "factor" number of times
        for (int j = 0; j < factor; j++)
        {
            // Iterate over pixels in scanline
            for (int k = 0; k < bi.biWidth; k++)
            {
                // Temporary storage
                RGBTRIPLE triple;

                // Read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

                // Write RGB triple to outfile "factor" number of times
                for (int l = 0; l < factor; l++)
                {
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                }
            }

            // Skip over padding in original BMP, if any
            fseek(inptr, padding, SEEK_CUR);

            // Add new padding to outfield
            for (int k = 0; k < newPadding; k++)
            {
                fputc(0x00, outptr);
            }

            // Go back to the start of the scanline if it needs to be reread and rewritten
            if (j < (factor - 1))
            {
                fseek(inptr, -(bi.biWidth * sizeof(RGBTRIPLE)) - padding, SEEK_CUR);
            }
        }
    }

    // Close infile
    fclose(inptr);

    // Close outfile
    fclose(outptr);

    // Success
    return 0;
}