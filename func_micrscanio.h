void *micrscan_init(void *arglist);
void *micrscan_controlloop(void *arglist);
void *discretisefullslide(double *fullimage, mcr *DATA);
void *movemicroscope(int nframes, int direction, mcr *DATA, int curr_row, int curr_col);
void addIMGtoSeenList(mcr *DATA, mIMG *seenIMG);
mIMG *acquireimage(mcr *DATA, int row, int col);
double HeuristicOnTissue(mIMG *image, mcr *DATA);
