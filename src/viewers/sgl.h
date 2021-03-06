/*****************************
Copyright 2017 Rafael Muñoz Salinas. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Rafael Muñoz Salinas ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Rafael Muñoz Salinas OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of Rafael Muñoz Salinas.
********************************/
//IF YOU USE THIS CODE, PLEASE CITE
//"Automatic generation and detection of highly reliable fiducial markers under occlusion"
//http://www.sciencedirect.com/science/article/pii/S0031320314000235
//AND
//"Generation of fiducial marker dictionaries using mixed integer linear programming"
//http://www.sciencedirect.com/science/article/pii/S0031320315003544

#ifndef _SGL_H
#define _SGL_H
#include <memory>
#include <cstring>
#include <vector>
#include <cmath>
#include <iostream>
#include <vector>

//Simplest graphics library
//Version 1.0.2
//Author. Rafael Muñoz Salinas (rmsalinas@uco.es) 2017

//Extremmely simple yet efficient device independent graphic library for points and lines
namespace sgl{
struct Point3{
    inline Point3( ){ }
    inline Point3(float X,float Y,float Z){x=X;y=Y;z=Z;}
    inline float norm()const{return sqrt( (x*x)+(y*y)+(z*z));}
    inline Point3 operator-(const Point3 &p)const{return Point3( x-p.x, y-p.y,z-p.z);}
    inline Point3 operator+(const Point3 &p)const{return Point3( x+p.x, y+p.y,z+p.z);}
    inline Point3 operator*(float v)const{return Point3( x*v,y*v,z*v);}
    float x,y,z;
    friend std::ostream & operator<<(std::ostream &str, const Point3 &p){
        str<<"["<<p.x<<" "<<p.y<<" "<<p.z<<"]";
        return str;
    }
};
template<typename T>
struct Point2_{
    Point2_( ){ }
    Point2_(float X,float Y){x=X;y=Y;}
    T x,y;
    friend std::ostream & operator<<(std::ostream &str, const Point2_<T> &p){
        str<<"["<<p.x<<" "<<p.y<<"]";
        return str;
    }
};
using Point2=Point2_<float>;
struct Matrix44{

    inline Matrix44 (){float m[16]={1,0,0,0, 0,1,0,0 ,0,0,1,0 ,0,0,0,1};std::memcpy(m44,m,16*sizeof(float));}
    inline Matrix44 (float m[16]){ std::memcpy(m44,m,16*sizeof(float));}
    inline const Matrix44&operator=(const Matrix44&M){std::memcpy(m44,M.m44,16*sizeof(float));return *this;}
    inline   void  rotateX(float rads){
        float c=cos(rads),s=sin(rads);
        float m[16]={1,0,0,0, 0,c,-s,0, 0,s,c,0, 0,0,0,1};
        (*this)= Matrix44(m)*(*this);
    }
    inline   void  rotateY(float rads){
        float c=cos(rads),s=sin(rads);
        float m[16]={c,0,s,0, 0,1,0,0 ,-s,0,c,0 ,0,0,0,1};
        (*this)= Matrix44(m)*(*this);
    }

    inline   void  rotateZ(float rads){
        float c=cos(rads),s=sin(rads);
        float m[16]={c,-s,0,0, s,c,0,0 ,0,0,1,0 ,0,0,0,1};
        (*this)= Matrix44(m)*(*this);
    }
    inline   void  translate(const Point3 &p){
        float m[16]={1,0,0,p.x, 0,1,0,p.y ,0,0,1,p.z ,0,0,0,1};
        (*this)= Matrix44(m)*(*this);
     }
    inline   Point3 operator*(const Point3 &p){    return Point3( p.x*m44[0]+ p.y*m44[1]+p.z*m44[2]+m44[3], p.x*m44[4]+ p.y*m44[5]+p.z*m44[6]+m44[7],p.x*m44[8]+ p.y*m44[9]+p.z*m44[10]+m44[11] );}
    inline  Matrix44 operator*(const Matrix44 &M){
        Matrix44 res;
        for(int i=0;i<3;i++)
            for(int j=0;j<4;j++){
                res(i,j)=0;
                for(int k=0;k<4;k++)
                    res(i,j)+= at(i,k)*M(k,j);
            }
        return res;
    }


    //access to elements
    inline float & operator()(int r,int c){return m44[r*4+c];}
    inline float   operator()(int r,int c)const{return m44[r*4+c];}
    inline float & at(int r,int c){return m44[r*4+c];}


    friend std::ostream & operator<<(std::ostream &str, const Matrix44 &m){
        for(int i=0;i<4;i++){
            for(int j=0;j<4;j++)
                str<<m(i,j)<<" ";
            str<<std::endl;
        }
        return str;
    }

    float m44[16];
};



struct Color{
    Color(){}
    Color(unsigned char  r,unsigned char  g,unsigned char b){rgb[0]=r;rgb[1]=g;rgb[2]=b;}
    inline unsigned char &operator[](int i){return rgb[i];}
    inline unsigned char operator[](int i)const{return rgb[i];}
    unsigned char rgb[3];
};


class Scene{

    Matrix44 _viewMatrix;
    std::vector<Matrix44> modelMatrices;
    Matrix44 _cameraMatrix;
    Matrix44 TM;//transform from global coordinates to camera ones
    Color *colorbuffer=0;
    bool mustFreeBuffer=true;

    float _focal;
    int _width,_height;

public:
    Scene(){
        modelMatrices.resize(1);
        modelMatrices[0]= Matrix44();//set identity
        TM=_viewMatrix;
    }

    ~Scene(){if (mustFreeBuffer&& colorbuffer) delete colorbuffer;}

    //sets external buffer

    inline void setCameraParams(float focal,int width,int height,void *external_buffer=0){
        _focal=focal;
        _width=width;
        _height=height;
        _cameraMatrix(0,0)=_focal*float(_width);
        _cameraMatrix(1,1)=_focal*float(_width);
        _cameraMatrix(0,2)=float(_width)/2.;
        _cameraMatrix(1,2)=float(_height)/2.;


        if (colorbuffer!=0 && mustFreeBuffer ) {
            delete colorbuffer;
            colorbuffer=0;
        }

        if(external_buffer!=0){
            colorbuffer=(Color*)external_buffer;
            mustFreeBuffer=false;
        }
        else{
            colorbuffer=new Color[_width*_height*3];
            mustFreeBuffer=false;
        }
    }
    //this will erase the transform matrix
    inline void setViewMatrix(const Matrix44&vp){_viewMatrix=vp; TM=_viewMatrix*modelMatrices.back();}

    inline void setModelMatrix(const Matrix44 &M=Matrix44()){
        modelMatrices.resize(1);
        modelMatrices[0]=M;
        TM=_viewMatrix*modelMatrices.back();
    }
    inline void pushModelMatrix(const Matrix44 &M=Matrix44()){
        modelMatrices.push_back(modelMatrices.back()*M);
        TM=_viewMatrix*modelMatrices.back();
    }
    inline void popModelMatrix(){
        if (modelMatrices.size()>1){
            modelMatrices.pop_back();
            TM=_viewMatrix*modelMatrices.back();
        }
    }



    inline void clear(const Color &backgroundColor){

        const int size=_width*_height;
        for(int i=0;i<size;i++) colorbuffer[i]=backgroundColor;
      }
    //returns current view point
    inline Matrix44& getViewMatrix(){return _viewMatrix;}
    inline Matrix44& getModelMatrix(){return modelMatrices.back();}

    //draws a 3d point
    inline void drawPoint(const Point3 &p,const Color &c,int size=1){  drawPoint(&p,c,size);}
    inline void drawPoints(const std::vector<Point3> &vp,const Color &c,int size=1){
        for(size_t i=0;i<vp.size();i++){
            Point3 pres=TM*vp[i];
            if (pres.z>0){
                pres=_cameraMatrix*pres;
                float invz=1./pres.z;
                pres.x*=invz;pres.y*=invz;
                if ( pres.x>= 0 &&  pres.x<_width && pres.y>=0 && pres.y<_height)
                    colorbuffer[int(pres.y)*_width+int(pres.x)]=c;
            }
        }

    }

    inline void drawPoints(const std::vector<Point3> &vp,const std::vector<Color> &vc,int size=1){
        auto M=_cameraMatrix*TM;
        for(size_t i=0;i<vp.size();i++){
            Point3 pres=M*vp[i];
            if (pres.z>0){
                pres=_cameraMatrix*pres;
                float invz=1./pres.z;
                pres.x*=invz;pres.y*=invz;
                if ( pres.x>= 0 &&  pres.x<_width && pres.y>=0 && pres.y<_height)
                    colorbuffer[int(pres.y)*_width+int(pres.x)]=vc[i];
            }
        }
    }
    inline void drawPoint(const Point3 *p,const Color &c,int size=1){
        Point2 fpres;
        Point3 p3d=TM*(*p);
        if ( p3d.z<0 ) return;
        if ( project(p3d,fpres)){
            Point2_<int> ipres(fpres.x,fpres.y);
            switch (size) {
            case 1:
                if ( ipres.x>=0 && ipres.x<_width && ipres.y>=0 && ipres.y<_height){
                    colorbuffer[ipres.y*_width+ipres.x]=c;
                }
                break;
            case 2:
                if ( ipres.x>=0 && ipres.x<_width-1 && ipres.y>=0 && ipres.y<_height-1){
                    auto ptr= &colorbuffer[ipres.y*_width+ipres.x];
                    ptr[0]=c;ptr[1]=c;
                    ptr+=_width;
                    ptr[0]=c;ptr[1]=c;
                }
                break;

            case 3:
                if ( ipres.x> 0 && ipres.x<_width-1 && ipres.y>0 && ipres.y<_height-1){
                    int stx=ipres.x-1;
                    int sty=(ipres.y-1)*_width;
                    colorbuffer[sty+stx]=c;colorbuffer[sty+stx+1]=c;colorbuffer[sty+stx+2]=c;
                    sty=(ipres.y)*_width;
                    colorbuffer[sty+stx]=c;colorbuffer[sty+stx+1]=c;colorbuffer[sty+stx+2]=c;
                    sty=(ipres.y+1)*_width;
                    colorbuffer[sty+stx]=c;colorbuffer[sty+stx+1]=c;colorbuffer[sty+stx+2]=c;
                }break;
            default:{
                int minX=ipres.x-size;if(minX<0)minX=0;
                int minY=ipres.y-size;if(minY<0)minY=0;
                int maxX=ipres.x+size;if(maxX>=_width)maxX=_width-1;
                int maxY=ipres.y+size;if(maxY>=_height)maxY=_height-1;
                for(int y=minY;y<maxY;y++){
                    auto *line=&colorbuffer[y];
                    for(int x=minX;x<maxX;x++) line[x]=c;
                }

            }
            }
        }
        (void)size;
    }

    inline void drawLine(const Point3 &p1,const Point3 &p2,const Color &color,int size=1){drawLine(&p1,&p2,color,size);}
    inline void drawLine(const Point3 *p1,const Point3 *p2,const Color &color,int size=1){
        Point3 p1t=TM*(*p1); if ( p1t.z<0 ) return;
        Point3 p2t=TM*(*p2);  if(p2t.z<0) return;//check that both are in front of camera(otherwise, do not draw)
        Point2 pr1,pr2;
        if(! project(p1t,pr1))return;
        if(! project(p2t,pr2))return;
        drawline(pr1,pr2,color,size);//project line bweten projected points
    }

    //returns the internal frame buffer
    inline unsigned char* getBuffer()const{return (unsigned char*)colorbuffer;}

    inline int getWidth()const{return _width;}
    inline int getHeight()const{return _height;}
private:


    inline bool project(const Point3 &p,Point2 &res){
        Point3 pres=_cameraMatrix*(p);
        if(pres.z==0) return false;
        float invz=1./pres.z;
        res.x=invz*pres.x+0.5; res.y=invz*pres.y+0.5;
        return true;
    }

    //Bresenham's algorithm

    inline bool inLimits(int x,int y){
        if (x<0)return false;
        if (y<0)return false;
        if(x>=_width)return false;
        if(y>=_height)return false;
        return true;
    }

    void drawline(Point2 start, Point2 end,  const Color& color ,int size=1)
    {


        (void)size;
        int x0=start.x,y0=start.y,x1=end.x,y1=end.y;

        if (!inLimits(x0,y0) && !inLimits(x1,y1) ) return;
        int dx = abs(x1 - x0), dy = abs(y1 - y0);
        int sx = (x0 < x1) ? 1 : -1, sy = (y0 < y1) ? 1 : -1;
        int err = dx - dy;
        while (true)
        {
            if ( size==1){
                if(y0>=0 && x0>=0 && y0<_height && x0<_width)   colorbuffer[y0*_width+x0]=color;
            }
            else{
                    if(y0>0 && x0>0 && y0<_height-1 && x0<_width-1)   {
                        auto start=&colorbuffer[(y0-1)*_width+x0-1];
                        start[0]=start[1]=start[2]=color;
                        start=&colorbuffer[y0*_width+x0-1];
                        start[0]=start[1]=start[2]=color;
                        start=&colorbuffer[(y0+1)*_width+x0-1];
                        start[0]=start[1]=start[2]=color;

                    }
            }
            if (x0 == x1 && y0 == y1) return;
            int e2 = (err << 1);
            if (e2 > -dy){err -= dy;x0 += sx;}
            if (e2 < dx){err += dx;y0 += sy;}
        }
    }
};




//draws the scene when required
class Drawer{

public:
    virtual void draw(sgl::Scene &scn)=0;
};
}

#ifndef NO_OPENCV
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
namespace sgl{
namespace opencv{


//Class using an opencv window to render and manipulate a sgl scene
class Viewer  {
    std::shared_ptr<Drawer> _Drawer;
    sgl::Scene _Scene;
    std::string _wname;
    float _f;
    int _w,_h;
    cv::Mat _imshow;
    int mode=1;//mode 0:big 3d, 1 big image
    bool showingHelp=false;
    bool canLeave=true;
    cv::Mat _resizedInImage;
    int snapShotIndex=0;
public:

    void setParams(std::shared_ptr<Drawer> drawer,float f,int width,int height,std::string wname){
        _Drawer=drawer;
        _imshow.create(height,width,CV_8UC3);
        assert(_imshow.isContinuous());
        _Scene.setCameraParams(f,width,height,_imshow.ptr<uchar>(0));
        _f=f;
        _w=width;_h=height;
        _wname=wname;
        cv::namedWindow(_wname,cv::WINDOW_NORMAL);
        cv::resizeWindow(_wname,width,height);
        cv::setMouseCallback(_wname, &Viewer::mouseCallBackFunc , this);
    }

    int exec(){
        //first creation of the image
        int k;
        drawScene();
        do{
            bool redraw=false;
            cv::imshow(_wname,_imshow);
            k=cv::waitKey(0);


            if(k=='h'){
                showingHelp=!showingHelp;
                redraw=true;
            }

            else if (k=='w'){
                std::string name="ucoslam-"+std::to_string(snapShotIndex++)+".png";
                cv::imwrite(name,_imshow);
                std::cerr<<"Image saved to "<<name<<std::endl;

            }

            if (redraw) drawScene();
        } while( k!=27);
        return k;
    }



protected:

    void drawScene( ){
        _Drawer->draw(_Scene);
        printHelp();
    }
    void   printHelp(){

        //print help commands
        if(!showingHelp)
            cv::putText(_imshow, "'h' showhelp", cvPoint(30,20), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.5, cvScalar(200,200,250), 1, CV_AA);
        else{
            cv::putText(_imshow, "'h' hide help", cvPoint(30,20), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.5, cvScalar(200,200,250), 1, CV_AA);
            cv::putText(_imshow, "'MOUSE[+SHIFT|CTRL]' change view", cvPoint(30,40), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.5, cvScalar(200,200,250), 1, CV_AA);
            cv::putText(_imshow, "'w' save image to file", cvPoint(30,60), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.5, cvScalar(200,200,250), 1, CV_AA);

        }
    }


    struct mouseInfo{
        sgl::Point2 pos;
        bool isTranslating=false,isZooming=false,isRotating=false;
    }mi;


    static   void mouseCallBackFunc(int event, int x, int y, int flags, void* userdata){
         Viewer *Sv=( Viewer*)userdata;
        bool redraw=false;
        if  ( event == cv::EVENT_LBUTTONDOWN ){
            Sv->mi.isRotating=Sv->mi.isTranslating=Sv->mi.isZooming=false;
            if ( flags&cv::EVENT_FLAG_CTRLKEY)
                Sv->mi.isZooming=true;
            else if ( flags&cv::EVENT_FLAG_SHIFTKEY) Sv->mi.isTranslating=true;
            else Sv->mi.isRotating=true;
        }
        else if  ( event == cv::EVENT_MBUTTONDOWN ) Sv->mi.isTranslating=true;
        else if ( event == cv::EVENT_LBUTTONUP ) {              Sv->mi.isRotating=Sv->mi.isTranslating=Sv->mi.isZooming=false;
        }
        else if ( event == cv::EVENT_MBUTTONUP ) Sv->mi.isTranslating=false;
        else if ( event == cv::EVENT_MOUSEMOVE )
        {
            sgl::Point2  dif(Sv->    mi.pos.x-x,Sv->   mi.pos.y-y);
            sgl::Matrix44 tm;//=Sv->_Scene.getTransformMatrix();

            if (Sv->mi.isRotating){
                tm.rotateX(-float(dif.y)/100);
                tm.rotateY(float(dif.x)/100);
            }
            else if (Sv->mi.isZooming){
                auto vp=Sv->_Scene.getViewMatrix();
                vp.translate({0,0, float(-dif.y*0.01)});
                Sv->_Scene.setViewMatrix(vp);
                redraw=true;
            }
            else if (Sv->mi.isTranslating){
                auto vp=Sv->_Scene.getViewMatrix();
                vp.translate(sgl::Point3(float(-dif.x)/100, float(-dif.y)/100,0.f));
                Sv->_Scene.setViewMatrix(vp);
                redraw=true;
            }
            if (Sv->mi.isRotating||Sv->mi.isZooming ||Sv->mi.isTranslating)  {
                sgl::Matrix44 res= tm*Sv->_Scene.getModelMatrix() ;
                Sv->_Scene.setModelMatrix(res);
                redraw=true;
            }
        }
        Sv->mi.pos=sgl::Point2(x,y);
        if (redraw)     {
            Sv->drawScene();
            cv::imshow(Sv->_wname,Sv->_imshow);
        }
    }
};
}
}
#endif


#endif
