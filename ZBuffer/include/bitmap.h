//
//  bitmap.h
//  ZBuffer
//
//  Created by 孟文龙 on 2021/11/12.
//

#ifndef bitmap_h
#define bitmap_h

struct Bitmap {
    unsigned char   *pixels;
    int     x, y;
    void    *dataBlock;
    
    Bitmap( int width, int height, void *d = NULL ) {
        pixels = new unsigned char[width * height * 4];
        x = width;
        y = height;
        dataBlock = d;
    }
    
    ~Bitmap() {
        delete [] pixels;
    }
    
    unsigned char* get_ptr( void ) const { return pixels; }
    long image_size( void ) const { return x * y * 4; }
};


#endif /* bitmap_h */
