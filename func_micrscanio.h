void *micrscan_init(void *arglist);
void *discretisefullslide(double *fullimage, mcr *DATA);
void *micrscan_drivemicroscope(void *arglist);
void addIMGtoSeenList(mcr *DATA, mIMG *seenIMG);
mIMG *acquireimage(mcr *DATA, int row, int col);
double HeuristicOnTissue(mIMG *image, mcr *DATA);
void *micrscan_AnalyseBufferImages(void *arglist);
void *micrscan_DecideNextMove(void *arglist);
void *micrscan_testimage(void *arglist);
void addIMGtoBuffer(mcr *MCR, mIMG *seenIMG);