Mat Binarization::niblackBinarization(const Mat& src, int size, double k)//fast
{
    Mat dst = (Mat_<unsigned char>(src.rows,src.cols));
    double sum=0;
    double sqSum=0;
    list<int> sumCols;
    list<int> sqSumCols;
    double mean;
    double varSum;
    double stdDev;
    double thresh;
    int regionStart=-floor(size/2);
    int regionEnd=ceil(size/2);
    for (int xOff=regionStart; xOff<regionEnd; xOff++)
    {
        double sumCol=0;
        double sqSumCol=0;
        for (int yOff=regionStart; yOff<regionEnd; yOff++)
        {
            int ty = 0+yOff;
            if (ty<0 || ty>=src.rows)
                ty = 0-yOff;
            
            int tx = 0+xOff;
            if (tx<0 || tx>=src.cols)
                tx = 0-xOff;
            int val = src.at<unsigned char>(ty,tx);
            sumCol +=val;
            sqSumCol +=val*val;
        }
        sumCols.push_back(sumCol);
        sum += sumCol;
        sqSumCols.push_back(sqSumCol);
        sqSum += sqSumCol;
    }
    mean=sum/(size*size);
    
    for (int y=0; y<src.rows; y++)
    {
        int x = y%2==0? 0 : src.cols-1;
        int xIter = y%2==0? 1 : -1;
        if (y!=0)
        {
            //move rows
            auto sumColsIter = sumCols.begin();
            auto sqSumColsIter = sqSumCols.begin();
            int yOffPrev = regionStart-1;
            int yOffNext = regionEnd-1;
            sum=0;
            sqSum=0;
            for (int xOff=regionStart; xOff<regionEnd; xOff++)
            {
                int tyPrev = y+yOffPrev;
                if (tyPrev<0 || tyPrev>=src.rows)
                    tyPrev = y-yOffPrev;
                int tyNext = y+yOffNext;
                if (tyNext<0 || tyNext>=src.rows)
                    tyNext = y-yOffNext;
                
                int tx = x+xOff;
                if (tx<0 || tx>=src.cols)
                    tx = x-xOff;
                
                int prevVal = src.at<unsigned char>(tyPrev,tx);
                int nextVal = src.at<unsigned char>(tyNext,tx);
                *sumColsIter -=prevVal;
                *sumColsIter +=nextVal;
                sum += *sumColsIter;
                sumColsIter++;
                
                *sqSumColsIter -=prevVal*prevVal;
                *sqSumColsIter +=nextVal*nextVal;
                sqSum += *sqSumColsIter;
                sqSumColsIter++;
            }
            mean=sum/(size*size);
        }
        varSum = (size*size)*mean*mean + sqSum -2*mean*sum;
        stdDev = sqrt(varSum/(size*size));
        thresh = mean + k*stdDev;
        //cout << "thrsh V " << thresh << "  mean: " << mean << "  varSum: " << varSum << "  sum: "<<sum<<"  sqSum: "<<sqSum<<endl;
        //assert(varSum>=0);
        if (src.at<unsigned char>(y,x) < thresh)
            dst.at<unsigned char>(y,x) = 1;//fg
        else
            dst.at<unsigned char>(y,x) = 0;//bg
        
        
        do 
        {
            x+=xIter;
            
            int xOff;
            if (y%2==0)
            {
                sum -= sumCols.front();
                sumCols.pop_front();
                
                sqSum -= sqSumCols.front();
                sqSumCols.pop_front();
                
                xOff = regionEnd-1;
            }
            else
            {
                sum -= sumCols.back();
                sumCols.pop_back();
                
                sqSum -= sqSumCols.back();
                sqSumCols.pop_back();
                
                xOff = regionStart;
            }
            
            double sumCol=0;
            double sqSumCol=0;
            for (int yOff=regionStart; yOff<regionEnd; yOff++)
            {
                int ty = y+yOff;
                if (ty<0 || ty>=src.rows)
                    ty = y-yOff;
                
                int tx = x+xOff;
                if (tx<0 || tx>=src.cols)
                    tx = x-xOff;
                
                int val = src.at<unsigned char>(ty,tx);
                sumCol += val;
                sqSumCol +=val*val;
            }
            if (y%2==0)
            {
                sumCols.push_back(sumCol);
                sqSumCols.push_back(sqSumCol);
            }
            else
            {
                sumCols.push_front(sumCol);
                sqSumCols.push_front(sqSumCol);
            }
            sum += sumCol;
            sqSum += sqSumCol;
            
            mean = sum/(size*size);
            varSum = (size*size)*mean*mean + sqSum -2*mean*sum;
            stdDev = sqrt(varSum/(size*size));
            thresh = mean + k*stdDev;
            //cout << "thrsh H " << thresh << "  mean: " << mean << "  varSum: " << varSum << "  sum: "<<sum<<"  sqSum: "<<sqSum<<endl;
            //assert(varSum>=0);
            if (src.at<unsigned char>(y,x) <= thresh)
                dst.at<unsigned char>(y,x) = 1;//fg
            else
                dst.at<unsigned char>(y,x) = 0;//bg
        } while (0<=x && x<src.cols);
    }
    return dst;
}
