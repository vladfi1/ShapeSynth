//
//  Matlab.h
//  
// Copyright (c) 2013-2014 Melinos Averkiou <m.averkiou@cs.ucl.ac.uk>

#ifndef MATLAB_H
#define MATLAB_H

#include "engine.h"
#include "matrix.h"

class Matlab {
        
public:
    
    struct MatlabVariable
    {
        double* data_ = 0;
        int nRows_ = 0;
        int nColumns_ = 0;
        std::string name_;
        
        ~MatlabVariable()
        {
            if(data_)
            {
                free(data_);
                data_ = 0;
            }
        }
    };
        
    static void init()
    {
        matlabEngine_ = 0;
    }
    
    static void openEngine()
    {
        // Open Matlab in case it was not opened
        if (!matlabEngine_)
        {
        //#if defined (APPLE)
            if (!(matlabEngine_ = engOpen( MATLAB_APP_PATH.c_str())))
        //#elif defined (UNIX)
        //    if (!(matlabEngine_ = engOpen("/usr/local/MATLAB/R2012a/bin/matlab")))
        //#endif
            {
                qCritical() << "Can't start MATLAB engine" ;
            }
            engSetVisible(matlabEngine_, true);
        }
        else
        {
            engEvalString(matlabEngine_, "clear all");
        }
    }
    
    static void closeEngine()
    {
        if(matlabEngine_)
        {
            engClose(matlabEngine_);
        }
    }
    
    static void runCode(const std::vector<MatlabVariable>& _ins, std::vector<MatlabVariable>& _outs, const std::string& _code)
    {
        openEngine();
        
        std::vector<mxArray*> inVarArrays;
        
        std::vector<MatlabVariable>::const_iterator itInVars(_ins.begin()), inVarsEnd(_ins.end());
        std::vector<MatlabVariable>::iterator itOutVars(_outs.begin()), outVarsEnd(_outs.end());
        
        for( ; itInVars!=inVarsEnd; ++itInVars )
        {
            // Create a 0-by-0 mxArray; will allocate the memory dynamically
            mxArray* cInVar = mxCreateNumericMatrix(0, 0, mxDOUBLE_CLASS, mxREAL);
            
            // Now allocate some memory to copy the values from this in variable so we can afterwards destroy the mxarray (and this memory) without hurting the initial in variable's data_ field
            double* cInData = (double*) mxMalloc( itInVars->nRows_ * itInVars->nColumns_ * sizeof(double));
            
            for (int i=0; i < itInVars->nRows_ * itInVars->nColumns_ ; i++)
            {
                cInData[i] = itInVars->data_[i];
            }
            
            // Put the C++ array into the mxArray and define its dimensions
            mxSetPr(cInVar, cInData);
            mxSetM(cInVar, itInVars->nRows_); //number of rows
            mxSetN(cInVar, itInVars->nColumns_); //number of columns
            
            engPutVariable(matlabEngine_, itInVars->name_.c_str(), cInVar);
            inVarArrays.push_back(cInVar);
        }
        
        std::string codeCopy(_code);
        codeCopy += " ";
        
        // Seems a bit unnecessary to add this extra code just to get the size of each out variable, but lets try it
        for( ; itOutVars!=outVarsEnd; ++itOutVars)
        {
            std::string sizeName = itOutVars->name_ + "_size";
        
            std::string addedCode = sizeName + " = size(" + itOutVars->name_ + "); ";
        
            codeCopy += addedCode;
        }
        
        // Evaluate the code
        engEvalString(matlabEngine_, codeCopy.c_str());
        
        itOutVars = _outs.begin();
        // Get the values for each out variable
        for( ; itOutVars!=outVarsEnd; ++itOutVars)
        {
            std::string sizeName = itOutVars->name_ + "_size";

            mxArray* cOutSizeVar = engGetVariable(matlabEngine_,sizeName.c_str());
            
            double* cOutSizeData = mxGetPr(cOutSizeVar);
            
            if(!cOutSizeData)
            {
                qCritical() << "Error while trying to get the size of " << QString(itOutVars->name_.c_str()) << ". Variable containing the size seems to be NULL!" ;
                continue;
            }
            // Hopefully cOutSizeData will contain the number of rows and columns of the specified out variable
            int nrows = cOutSizeData[0];
            int ncols = cOutSizeData[1];
            
            itOutVars->nRows_ = nrows;
            itOutVars->nColumns_ = ncols;
            
            mxArray* cOutVar = engGetVariable(matlabEngine_,itOutVars->name_.c_str());
            
            double* cOutData =  mxGetPr(cOutVar);
            
            // The out variable's data_ field should not point to any data, but just is case it does, delete it and give a message
            if(itOutVars->data_)
            {
                qCritical() << "Out variable's " << QString(itOutVars->name_.c_str()) << " data field seems to point to some memory although it shouldn't. Deallocating this memory!" ;
                
                free(itOutVars->data_);
            }
            
            // Now allocate some memory to put the values for this out variable and copy the values from the mxarray so we can then destroy it
            itOutVars->data_ = new double[nrows*ncols]; //*sizeof(double));

            for (int i=0; i<nrows*ncols; i++)
            {
                itOutVars->data_[i] = cOutData[i];
            }
            
            // Done copying, we can now destroy the mxarray for the out variable as well as the mxarray for the out variable's size
            mxDestroyArray(cOutVar);
            mxDestroyArray(cOutSizeVar);
        }
        
        // Go over the allocated inVarArrays and destroy them. This should also delete the memory associated with every array (all the cInData's) otherwise we will have memory leaks
        std::vector<mxArray*>::iterator itInVarArrays(inVarArrays.begin()), inVarArraysEnd(inVarArrays.end());
        
        for( ; itInVarArrays!=inVarArraysEnd; ++itInVarArrays)
        {
            mxDestroyArray(*itInVarArrays);
        }
        
    }
    
private:
    
    static Engine* matlabEngine_;
    
    Matlab()
    {
        
    }
    
    ~Matlab()
    {
        
    }

};

#endif
