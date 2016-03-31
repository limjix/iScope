//iScope Functions
void *micrscan_init(void *arglist);
void *micrscan_drivemicroscope(void *arglist);
void *micrscan_CreateMicrGraph(void *arglist);
void *micrscan_AnalyseBufferImages(void *arglist);
void *micrscan_DecideNextMove(void *arglist);

//Utilities
void *discretisefullslide(double *fullimage, mcr *MCR);
void addIMGtoSeenList(mcr *MCR, mIMG *seenIMG);
void addIMGtoBuffer(mcr *MCR, mIMG *seenIMG);
mIMG *acquireimage(mcr *MCR, int row, int col);
nodes *QueryGraph(hgph *graph, int row, int col);
double HeuristicOnTissue(mIMG *image, mcr *MCR);

void mimicthresholding();

//Stuff For the actual thing
void *micrscan_MeanImageHeuristic(void *arglist);
void thresholding();

//DEBUGGING PRINT
void *micrscan_GraphStructToFile(void *arglist);
