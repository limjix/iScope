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

//DEBUGGING PRINT
void *micrscan_GraphStructToFile(void *arglist);

//Stuff For the actual microscope
double MeanImageHeuristic(rgb *image);
void *micrscan_AnalyseBuf(void *arglist);
void *UpdateLocation(void *arglist);
void *micrscan_scaninit(void *arglist);
