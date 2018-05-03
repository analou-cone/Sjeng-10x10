/* 
 * the front-end code
 *
 */

#include <SDL.h>
#include <SDL_image.h>

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <math.h>

#include "sjeng.h"
#include "extvars.h"
#include "protos.h"

static const char   *boardImageName = "chessboardalone-60.png";
static const float  boardRatio = 0.6;

// colours are RGBA Uint32's
static const Uint32 whiteColour = 0xffffffff;
static const Uint32 blackColour = 0x000000ff;
static const Uint32 redColour   = 0xff0000ff;
static const Uint32 greenColour = 0x00ff00ff;
static const Uint32 blueColour  = 0x0000ffff;
static const Uint32 yellowColour= 0xffff00ff;

struct sdl_data {
    SDL_Surface *screen;
    SDL_Surface	*boardImage;
    SDL_Surface *pieceGraphics[4][4][24];
    unsigned squareCenter[10][10][2];
    SDL_Rect srcRect;
    SDL_Rect dstRect;
};


struct rankFileData {
    int rank;
    int file;
    char rankFile[8];
};


void
setBoardHistory (int src[rankFileArraySize], int dst[rankFileArraySize]) {
    int i;

    if (src[32] == frame) {
        fprintf (stderr, " E Assert failed: src[32] is frame!\n");
    }

    // copy the src board position to the dest - used to record
    // positions of the board over time for playback
    for (i=0; i<rankFileArraySize; i++) {
        dst[i] = src[i];
    }
}


static void
sdlInitSquares (struct sdl_data *sdp)
{
    unsigned i, j, k;
    float sc[10][10][2];

    //fprintf (stderr, " - sdlInitSquares() beginning\n");

    sdp->squareCenter[0][9][0] = 232;  sdp->squareCenter[0][9][1] = 592;
    sdp->squareCenter[3][9][0] = 506;  sdp->squareCenter[3][9][1] = 436;
    sdp->squareCenter[6][9][0] = 744;  sdp->squareCenter[6][9][1] = 300;
    sdp->squareCenter[9][9][0] = 956;  sdp->squareCenter[9][9][1] = 174;

    sdp->squareCenter[0][6][0] = 394;  sdp->squareCenter[0][6][1] = 804;
    sdp->squareCenter[3][6][0] = 676;  sdp->squareCenter[3][6][1] = 614;
    sdp->squareCenter[6][6][0] = 934;  sdp->squareCenter[6][6][1] = 464;
    sdp->squareCenter[9][6][0] = 1154; sdp->squareCenter[9][6][1] = 316;

    sdp->squareCenter[0][3][0] = 592;  sdp->squareCenter[0][3][1] = 1068;
    sdp->squareCenter[3][3][0] = 894;  sdp->squareCenter[3][3][1] = 850;
    sdp->squareCenter[6][3][0] = 1150; sdp->squareCenter[6][3][1] = 662;
    sdp->squareCenter[9][3][0] = 1382; sdp->squareCenter[9][3][1] = 490;

    sdp->squareCenter[0][0][0] = 842;  sdp->squareCenter[0][0][1] = 1394;
    sdp->squareCenter[3][0][0] = 1156; sdp->squareCenter[3][0][1] = 1126;
    sdp->squareCenter[6][0][0] = 1433; sdp->squareCenter[6][0][1] = 900;
    sdp->squareCenter[9][0][0] = 1674; sdp->squareCenter[9][0][1] = 694;

    // the board is actually only boardRatio-sized
    for (i = 0; i < 10; i+=3)
        for (j = 0; j < 10; j+=3)
            for (k = 0; k < 2; k++)
                sc[i][j][k] = (float) sdp->squareCenter[i][j][k] * boardRatio;

    // first all unknown ranks on a/d/g/j
    //fprintf (stderr, " + Initialising squareCenters stage 1 (j in 1,2,4,5,7,8)\n");
    for (i = 0; i < 10; i+=3) {
        for (j = 0; j < 10 ; j++)  {
            for (k = 0; k < 2; k++) {
                if (j == 1 || j == 4 || j == 7) {
                    sc[i][j][k] = (sc[i][j-1][k] - (sc[i][j-1][k] - sc[i][j+2][k])*0.333);
                } else if (j == 2 || j == 5 || j == 8) {
                    sc[i][j][k] = (sc[i][j-2][k] - (sc[i][j-2][k] - sc[i][j+1][k])*0.666);
                }
            }
        }
    }

    // now all unknown files on 2/4/7/10
    //fprintf (stderr, " + Initialising squareCenters stage 2 (i in 1,2,4,5,7,8)\n");
    for (i = 0; i < 10; i++)  {
        for (j = 0; j < 10; j+=3) {
            for (k = 0; k < 2; k++) {
                if (i == 1 || i == 4 || i == 7) {
                    sc[i][j][k] = (sc[i-1][j][k] - (sc[i-1][j][k] - sc[i+2][j][k])*0.333);
                } else if (i == 2 || i == 5 || i == 8) {
                    sc[i][j][k] = (sc[i-2][j][k] - (sc[i-2][j][k] - sc[i+1][j][k])*0.666);
                }
            }
        }
    }

    // finally, we need to get the four squares between all known
    // centers. we can just iterate over every rank or file, but i'll
    // choose rank, because i can
    //fprintf (stderr, " + Initialising squareCenters stage 3\n");
    for (i = 0; i < 10 ; i++)  {
        for (j = 0; j < 10 ; j++)  {
            for (k = 0; k < 2; k++) {
                if (j == 1 || j == 4 || j == 7) {
                    sc[i][j][k] = (sc[i][j-1][k] - (sc[i][j-1][k] - sc[i][j+2][k])*0.333);
                } else if (j == 2 || j == 5 || j == 8) {
                    sc[i][j][k] = (sc[i][j-2][k] - (sc[i][j-2][k] - sc[i][j+1][k])*0.666);
                }
            }
        }
    }

    // convert the floats back to ints
    //fprintf (stderr, " + Copying back\n");
    for (i = 0; i < 10; i++)
        for (j = 0; j < 10; j++)
            for (k = 0; k < 2; k++)
                sdp->squareCenter[i][j][k] = (int) sc[i][j][k];
}


struct sdl_data *
sdlInit (void) {
    struct sdl_data *sdp;

    unsigned i;
    unsigned j;

    if (!(sdp = malloc(sizeof(*sdp)))) {
        fprintf (stderr, " E Couldn't create SDL data structure\n");
        return NULL;
    }
    bzero(sdp, sizeof(*sdp));

    // find square centers across board
    sdlInitSquares(sdp);

#ifdef __APPLE__
    {
        extern void NSApplicationLoad(void);
        NSApplicationLoad();
    }
#endif // __APPLE__

    if (SDL_Init (SDL_INIT_VIDEO) == -1) {
        fprintf (stderr, " E Couldn't init SDL\n");
        goto fail;
    }

    if (!(sdp->boardImage = IMG_Load(boardImageName))) {
        fprintf (stderr, " E Couldn't load %s: %s\n", boardImageName, IMG_GetError());
        goto fail;
    }

    atexit(SDL_Quit); //Now that we're enabled, make sure we cleanup

    SDL_WM_SetCaption("DecaChess Front-end GUI", NULL);

    //Create a resizable window with the board image size
    sdp->screen = SDL_SetVideoMode(sdp->boardImage->w,
        sdp->boardImage->h,
        32,
        SDL_HWSURFACE | SDL_RESIZABLE | SDL_DOUBLEBUF);

    if(!sdp->screen){
        fprintf (stderr, " E Couldn't create screen\n");
        goto fail;
    }

    return sdp;

fail:
    sdlFinish(sdp);
    return NULL;
}


void
sdlFinish (struct sdl_data *sdp) {
    if (sdp->boardImage)
        SDL_FreeSurface(sdp->boardImage);

    SDL_Quit();
    free(sdp);

    exit;
}


void
sdlDrawBoard (struct sdl_data *sdp, int *boardToDraw) {
    int drawFile;
    int drawRank;
    int boardLoc;
    int doFlip;

    int drawX;
    int drawY;

    //fprintf (stderr, " - sdlDrawBoard() beginning\n");

    // make a white-coloured window the size of the chessboard graphic
    SDL_Surface *chessPieceGraphic;
    SDL_FillRect(sdp->screen, NULL, whiteColour);
    SDL_BlitSurface(sdp->boardImage, NULL, sdp->screen, NULL);

    // find out where on the board the chesspieces are
    // then load graphics and put them on the board
    doFlip=0;
    for (drawFile=9; drawFile>=0; drawFile--) {
        for (drawRank=9; drawRank>=0; drawRank--) {
            boardLoc = plusRank * 2 + 2 + drawRank*plusRank + drawFile;
            if (boardToDraw[boardLoc] != npiece) {
                // put it on the board
                chessPieceGraphic = getPieceGraphic(sdp, boardToDraw, drawFile, drawRank);

                if (chessPieceGraphic != NULL) {
                    drawX = floor (sdp->squareCenter[drawFile][drawRank][0] - (chessPieceGraphic->w/2));
                    drawY = floor (sdp->squareCenter[drawFile][drawRank][1] - (chessPieceGraphic->h/2));

                    //fprintf (stderr, " - Piece Graphic w/h == %d/%d at x/y == %d/%d given squareCenter is %d,%d\n",chessPieceGraphic->w, chessPieceGraphic->h, drawX, drawY, sdp->squareCenter[drawFile][drawRank][0], sdp->squareCenter[drawFile][drawRank][1]);

                    // prepare for blitting the graphic onto the screen
                    sdp->srcRect.x=0;
                    sdp->srcRect.y=0;
                    sdp->srcRect.w=chessPieceGraphic->w;
                    sdp->srcRect.h=chessPieceGraphic->h;

                    sdp->dstRect.x=drawX;
                    sdp->dstRect.y=drawY;
                    sdp->dstRect.w=chessPieceGraphic->w;
                    sdp->dstRect.h=chessPieceGraphic->h;

                    if (SDL_BlitSurface(chessPieceGraphic, &sdp->srcRect, sdp->screen, &sdp->dstRect) == 0) {
                        // all is well
                        doFlip=1;
                    } else {
                        // hrm. some blitting problem?
                        // apparently -2 means we want to redraw the entire screen surface
                        // and -1 means basically "ARG!"
                        doFlip=0;
                    }
                } else {
                    assert (0);
                    // there's a bug that keeps coming up that causes segfaults where this location
                    // is a frame, not a piece... so getPieceGraphic is returning NULL - i want to
                    // dump out the chessboard in that situation, but am having issues doing that
                    fprintf (stderr, " E WTF - Couldn't get piece graphic. Dumping boardToDraw[]\n");
                    fprintf (stderr, " - globalBoardPos = %d, viewBoardPos = %d\n", globalBoardPos, viewBoardPos);
                    int i;
                    int j;
                    for (i=0; i<plusRank; i++) {
                        fprintf (stderr, "%02d: ", i);
                        for (j=0; j<plusRank; j++) {
                            fprintf (stderr, "%03d=%02d ", i*plusRank + j, boardToDraw[i*plusRank + j]);
                        }
                        fprintf (stderr, "\n");
                    }

                    return;
                }
            }
        }
    }
    if (doFlip) {
        SDL_Flip(sdp->screen);
        //SDL_UpdateRect (sdp->screen, 0, 0, 0, 0);
    } else {
        fprintf (stderr, " E Not SDL_Flip()'ing\n");
    }
}


SDL_Surface * 
getPieceGraphic (struct sdl_data *sdp, int *boardToDraw, unsigned getFile, unsigned getRank) {
    //fprintf (stderr, " - getPieceGraphic(sdp, %d, %d) beginning\n", getFile, getRank);

    // names of chess pieces - sjeng.h defines frame,wpawn...npiece
    char *chessPieceNames[16];
    chessPieceNames[frame]   = "frame";
    chessPieceNames[wpawn]   = "whitepawn";
    chessPieceNames[wknight] = "whiteknight";
    chessPieceNames[wbishop] = "whitebishop";
    chessPieceNames[wrook]   = "whiterook";
    chessPieceNames[wqueen]  = "whitequeen";
    chessPieceNames[wking]   = "whiteking";
    chessPieceNames[bpawn]   = "blackpawn";
    chessPieceNames[bknight] = "blackknight";
    chessPieceNames[bbishop] = "blackbishop";
    chessPieceNames[brook]   = "blackrook";
    chessPieceNames[bqueen]  = "blackqueen";
    chessPieceNames[bking]   = "blackking";
    chessPieceNames[npiece]  = "npiece";

    unsigned boardLoc;
    unsigned cbX;
    unsigned cbY;

    char *fileName;
    if ((fileName = malloc(256)) == NULL) {
        fprintf (stderr, " E Couldn't malloc() fileName\n");
        sdlFinish (sdp);
        return NULL;
    }

    // if there's a piece at that location on the board...
    boardLoc = plusRank * 2 + 2 + getRank*plusRank + getFile;
    if (boardToDraw[boardLoc] != npiece && boardToDraw[boardLoc] != frame) {
        char letterPart;
        if (getFile == 0 || getFile == 1) { letterPart = 'a'; cbX=0; }
        if (getFile == 2 || getFile == 3 || getFile == 4) { letterPart = 'd'; cbX=1; }
        if (getFile == 5 || getFile == 6 || getFile == 7) { letterPart = 'g'; cbX=2; }
        if (getFile == 8 || getFile == 9) { letterPart = 'j'; cbX=3; }

        unsigned numberPart;
        if (getRank == 0 || getRank == 1) { numberPart = 1; cbY=0; }
        if (getRank == 2 || getRank == 3 || getRank == 4) { numberPart = 4; cbY=1; }
        if (getRank == 5 || getRank == 6 || getRank == 7) { numberPart = 7; cbY=2; }
        if (getRank == 8 || getRank == 9) { numberPart = 10; cbY=3; }

        // load chess piece graphic off disk if we haven't already
        if (sdp->pieceGraphics[cbX][cbY][boardToDraw[boardLoc]] == NULL) {
            sprintf (fileName, "Assets/SinglePieces-50/%c%d-%s.png", letterPart, numberPart, chessPieceNames[boardToDraw[boardLoc]]);
            //fprintf (stderr, " + Loading Graphic %s into pieceGraphics[%d][%d]\n", fileName, cbX, cbY);
            if (!(sdp->pieceGraphics[cbX][cbY][boardToDraw[boardLoc]] = IMG_Load(fileName))) {
                fprintf (stderr, " E Couldn't load %s: %s\n", fileName, IMG_GetError());
                sdlFinish(sdp);
                return NULL;
            }
        } else {
            //fprintf (stderr, " - Already loaded %c%d's %s\n", letterPart, numberPart, chessPieceNames[boardToDraw[boardLoc]]);
        }

        free (fileName);

        return sdp->pieceGraphics[cbX][cbY][boardToDraw[boardLoc]];
    } else {
        fprintf (stderr, " E Hmm. At %d,%d is %s - There's no piece there!\n", getFile, getRank, chessPieceNames[boardToDraw[boardLoc]]);
        return NULL;
    }
}


void
sdlDealWithMouse (struct sdl_data *sdp, int inbtnX, int inbtnY, struct rankFileData *squareChosen) {
    // determine which square they clicked on
    int i;
    int j;

    int squareX;
    int squareY;

    float scX;
    float scY;
    float btX = (float) inbtnX;
    float btY = (float) inbtnY;

    float minDist = 99999999.0;
    for (i=0; i<10; i++) {
        for (j=0; j<10; j++) {
            // distance" formula. sqrt (d(x)^2 + d(y)^2)
            scX = (float) sdp->squareCenter[i][j][0];
            scY = (float) sdp->squareCenter[i][j][1];
            float dist = sqrt (pow(btX - scX, 2) + pow(btY - scY, 2));
            if (dist < minDist) {
                minDist = dist;
                squareX = i;
                squareY = j;
            }
        }
    }
    char file = 'a' + squareX;
    int rank = squareY + 1;

    // if (minDist > 55) {
    //     fprintf (stderr, " - (%d,%d) dist=%f - That's kinda far from anything, isn't it?\n", inbtnX, inbtnY, minDist);
    // }

    // fprintf (stderr, " - x,y==(%d,%d) square==%d,%d alpha==%c%d\n", inbtnX, inbtnY, squareX, squareY, file, rank);
    sprintf (squareChosen->rankFile, "%c%d", file, rank);
    squareChosen->rank = squareY;
    squareChosen->file = squareX;
}


// both file and rank should be base-0
void
sdlShowThink (struct sdl_data *sdp, int score, int f_file, int f_rank, int t_file, int t_rank) {
    float fromX;
    float fromY;
    float toX;
    float toY;

    Uint32 *pixelp;

    int thinkDotSize = 3;
    SDL_Surface *thinkDot;
    thinkDot = SDL_CreateRGBSurface (
            SDL_HWSURFACE,
            thinkDotSize,
            thinkDotSize,
            32,
            0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);

    fromX = (float) sdp->squareCenter[f_file][f_rank][0];
    fromY = (float) sdp->squareCenter[f_file][f_rank][1];

    toX = (float) sdp->squareCenter[t_file][t_rank][0];
    toY = (float) sdp->squareCenter[t_file][t_rank][1];

    // for mouse hover, the to/from are the same, so add some interest
    if (fromX == toX && fromY == toY) {
        toY -= 50;
    }

    // fprintf (stderr, " + sdlShowThink - %d, %d = %f, %d = %f, %d = %f, %d = %f\n", score, f_file, fromX, f_rank, fromY, t_file, toX, t_rank, toY);

    // how much to affect the colour of the arc
    int colourOffset = score * 2;

    // make the arc start at a bit random off-center
    int offCentreness = 2;

    // number of segments in arc
    float dist = sqrt ((pow (toX - fromX, 2)) + (pow (toY - fromY, 2)));
    int numSegments = (int) (dist / 5);

    int direction;
    int amt;

    // offset source and destination point slightly
    direction = (int) ((float)random() * 2 / RAND_MAX);
    amt       = ((float)random() * offCentreness / RAND_MAX);
    if (direction) { fromX += amt; } else { fromX -= amt; }
    direction = (int) ((float)random() * 2 / RAND_MAX);
    amt       = ((float)random() * offCentreness / RAND_MAX);
    if (direction) { fromY += amt; } else { fromY -= amt; }
    direction = (int) ((float)random() * 2 / RAND_MAX);
    amt       = ((float)random() * offCentreness / RAND_MAX);
    if (direction) { toX += amt; } else { toX -= amt; }
    direction = (int) ((float)random() * 2 / RAND_MAX);
    amt       = ((float)random() * offCentreness / RAND_MAX);
    if (direction) { toY += amt; } else { toY -= amt; }

    float currX = fromX;
    float currY = fromY;

    float dX = (toX - fromX) / numSegments;
    float dY = (toY - fromY) / numSegments;

    float accelX;
    // if the change in X is small, arc it right/left
    if (abs (fromX - toX) < 100) {
        accelX = ((fromX - sdp->squareCenter[4][4][0]) / sdp->screen->w * 2);
        if (accelX >= 0) { accelX += 1; } else { accelX -= 1; }
    } else {
        accelX = ((float)random() * (abs (dX)) / RAND_MAX);
    }

    // always arc Y in the upward direction since that's the perspective of
    // the board
    float accelY = ((float)random() * (abs (dX)) / RAND_MAX) - 3;

    // the dampening brings the arc back on track to the eventual target
    float dampX = accelX / numSegments * 2;
    float dampY = accelY / numSegments * 2;

    sdp->srcRect.w = thinkDotSize;
    sdp->srcRect.h = thinkDotSize;
    sdp->srcRect.x = 0;
    sdp->srcRect.y = 0;

    // pick the colour to display - clip it, SDL seems to dislike out-of-bounds colours
    int rColour = 120+colourOffset;
    int gColour = 120+colourOffset;
    int bColour = 100-colourOffset;
    if (rColour > 255) { rColour = 255; } else if (rColour < 0) { rColour = 0; }
    if (gColour > 255) { gColour = 255; } else if (gColour < 0) { gColour = 0; }
    if (bColour > 255) { bColour = 255; } else if (bColour < 0) { bColour = 0; }

    // a checkmate is RED
    if (score > 999000 || score < -999000) {
        rColour = 255;
        gColour = 0;
        bColour = 0;
    }

    // we now know the colour the dot will be
    Uint32 blotColour;
    blotColour = (((Uint8) rColour) <<24)
        + (((Uint8) gColour) <<16)
        + (((Uint8) bColour) <<8)
        + ((Uint8) 255); // alpha

    // fill the dot with the colour
    int i;
    int j;
    SDL_LockSurface(thinkDot);
    for (i=0; i<thinkDotSize*thinkDotSize; i++) {
        pixelp = (thinkDot->pixels + i * sizeof (Uint32));
        *pixelp = blotColour;
    }
    SDL_UnlockSurface(thinkDot);

    // add a white highlight and black shadow so you can always see the dot
    pixelp = thinkDot->pixels;
    *pixelp = whiteColour;
    pixelp = (thinkDot->pixels + ((thinkDotSize*thinkDotSize - 1) * sizeof (Uint32)));
    *pixelp = blackColour;

    // make an arc with the dots
    for (i = 0; i < numSegments; i++) {
        sdp->dstRect.w = thinkDotSize;
        sdp->dstRect.h = thinkDotSize;
        sdp->dstRect.x = (int) currX;
        sdp->dstRect.y = (int) currY;

        if (SDL_BlitSurface(thinkDot, &sdp->srcRect, sdp->screen, &sdp->dstRect) == 0) {
            SDL_UpdateRect (sdp->screen, (int) currX, (int) currY, thinkDotSize, thinkDotSize);
            //SDL_Flip(sdp->screen);
        } else {
            fprintf (stderr, "blitFail");
        }

        currX += dX + accelX;
        currY += dY + accelY;

        accelX -= dampX;
        accelY -= dampY;
    }
}


void
sdlInput (struct sdl_data *sdp, char *input) {
    SDL_Event event; //Events

    char *keyPressed;
    int fromChosen = 0;
    struct rankFileData fromRankFile;
    struct rankFileData toRankFile;
    struct rankFileData hoverRankFile;

    bool done = 0;
    while (!done) {
        while (SDL_PollEvent(&event)){
            switch (event.type){
                case SDL_QUIT: //User hit the X (or equivelent)
                    done = 1;
                    strncpy (input, "q", STR_BUFF);
                    return;
                    break;
                case SDL_VIDEORESIZE: //User resized window
                    /*
                    screen = SDL_SetVideoMode(event.resize.w, event.resize.h, 32,
                    SDL_HWSURFACE | SDL_RESIZABLE); // Create new window
                    */
                    break; //Event handled, fetch next :)
                case SDL_KEYDOWN:
                    keyPressed = SDL_GetKeyName (event.key.keysym.sym);
                    if (strncmp (keyPressed, "escape", STR_BUFF) == 0) {
                        if (automode) {
                            fprintf (stderr, " + Aborting automode.\n");
                            automode = 0;
                            strncpy (input, "", STR_BUFF);
                        } else {
                            fprintf (stderr, " + Quitting decachess! Enjoy the world.\n");
                            strncpy (input, "q", STR_BUFF);
                        }
                        return;
                    } else if (strncmp (keyPressed, "a", STR_BUFF) == 0) {
                        strncpy (input, "auto", STR_BUFF);
                        return;
                    } else if (strncmp (keyPressed, "1", STR_BUFF) == 0) {
                        strncpy (input, "sd 2", STR_BUFF);
                        return;
                    } else if (strncmp (keyPressed, "2", STR_BUFF) == 0) {
                        strncpy (input, "sd 4", STR_BUFF);
                        return;
                    } else if (strncmp (keyPressed, "3", STR_BUFF) == 0) {
                        strncpy (input, "sd 6", STR_BUFF);
                        return;
                    } else if (strncmp (keyPressed, "4", STR_BUFF) == 0) {
                        strncpy (input, "sd 8", STR_BUFF);
                        return;
                    } else if (strncmp (keyPressed, "5", STR_BUFF) == 0) {
                        strncpy (input, "sd 10", STR_BUFF);
                        return;
                    } else if (strncmp (keyPressed, "6", STR_BUFF) == 0) {
                        strncpy (input, "sd 12", STR_BUFF);
                        return;
                    } else if (strncmp (keyPressed, "g", STR_BUFF) == 0) {
                        strncpy (input, "go", STR_BUFF);
                        return;
                    } else if (strncmp (keyPressed, "left", STR_BUFF) == 0) {
                        viewBoardPos--;
                        if (viewBoardPos < 1) { viewBoardPos = 1; }
                        sdlDrawBoard (sdp, boardHistory[viewBoardPos]);
                    } else if (strncmp (keyPressed, "right", STR_BUFF) == 0) {
                        viewBoardPos++;
                        if (viewBoardPos > globalBoardPos) { viewBoardPos = globalBoardPos; }
                        sdlDrawBoard (sdp, boardHistory[viewBoardPos]);
                    } else if (strncmp (keyPressed, "page down", STR_BUFF) == 0) {
                        viewBoardPos -= 8;
                        if (viewBoardPos < 1) { viewBoardPos = 1; }
                        sdlDrawBoard (sdp, boardHistory[viewBoardPos]);
                    } else if (strncmp (keyPressed, "page up", STR_BUFF) == 0) {
                        viewBoardPos += 8;
                        if (viewBoardPos > globalBoardPos) { viewBoardPos = globalBoardPos; }
                        sdlDrawBoard (sdp, boardHistory[viewBoardPos]);
                    } else {
                        fprintf (stderr, " - unknown key [%s]\n", keyPressed);
                    }
                    break;
                case SDL_MOUSEMOTION:
                    sdlDealWithMouse (sdp, (int) event.motion.x, (int) event.motion.y, &hoverRankFile);
                    sdlShowThink (sdp, 999999, hoverRankFile.file, hoverRankFile.rank, hoverRankFile.file, hoverRankFile.rank);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (!fromChosen) {
                        sdlDealWithMouse (sdp, (int) event.button.x, (int) event.button.y, &fromRankFile);
                        if (/*&fromRankFile*/1) {
                            fromChosen = 1;
                        }
                    } else {
                        sdlDealWithMouse (sdp, (int) event.button.x, (int) event.button.y, &toRankFile);
                        if (/*&toRankFile*/1) {
                            sprintf (input, "%s%s", fromRankFile.rankFile, toRankFile.rankFile);
                            return;
                        }
                    }
                    break;
            } //Finished with current event
        } //Done with all events for now

        // if we're in automode, go back to main loop
        if (automode) { return; }

        // give the human a few milliseconds to decide what to do - yah, this is actually
        // necessary on Linux. without it, CPU goes to 100% - apparently the event loop
        // does *NOT* block (and i wouldn't want it to if it did, because then things like
        // automode wouldn't work)
        SDL_Delay(40);
    }
}

