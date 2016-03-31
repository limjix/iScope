typedef struct mvec
{
  //general-purpose container for message vector

	int length; //Length of vector
	unsigned sender; //Node that send message
	double* vector; //input vector values here.

	double *states; //USED ONLY FOR MARGINALS

} mvec;
