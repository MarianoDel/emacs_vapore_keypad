//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### PARAMETERS.H ##########################
//---------------------------------------------

#ifndef _PARAMETERS_H_
#define _PARAMETERS_H_


#define B1_AUDIO_MASK    0x01
#define B3_AUDIO_MASK    0x04
typedef struct parameters {

    // conf for Button1
    unsigned char b1t;
    unsigned char b1r;
    // conf for Button2
    unsigned char b2t;
    unsigned char b2r;
    // conf for Button3
    unsigned char b3t;
    unsigned char b3r;
    // conf for Button4
    unsigned char b4t;
    unsigned char b4r;
    // audio in Buttons
    unsigned char audio_buttons;

    unsigned short interdigit;
    unsigned short wait_for_code;

} parameters_typedef;


#endif    /* _PARAMETERS_H_ */

//--- end of file ---//

