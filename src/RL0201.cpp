/*
Recursion Lab (2012), by Andrew Barrette
Compile with g++ -o RL0201.exe RL0201.cpp EasyBMP.cpp glut32.lib -lopengl32 -lglu32 -static-libgcc -static-libstdc++
*/

#include<iostream>
#include<math.h>
#include<vector>
#include<cstring>
#include<windows.h>
#include "GL/glut.h"
#include<cstdio>
#include<cstdlib>
#include<time.h>
#include<fstream>
#include<new>
#include "EasyBMP.h"
using namespace std;

#define STD_FONT GLUT_BITMAP_HELVETICA_18
#define STD_FONT2 GLUT_BITMAP_HELVETICA_12
#define STD_FONT2_HEIGHT 12

GLint winw=900,winh=700,mouseloc[2],tbwidth=200,renderscreen[]={300,100,600,350},textoffset[]={4,4},origin[]={0,0};
GLfloat plotradius[]={100,100},ctemp[4],renderarea[2][2]={(-1,-1),(-1,-1)};
short fractaltype,toberend,helpmode=0;
bool renderareaselect=false;
float renderprogress,renderratio;

struct coordtype
{
	GLfloat x[2];
	bool recursive;
	bool free;
};

struct vecttype
{
	GLfloat x[2];
};

void plotvala(GLint winx,GLint winy,GLfloat *temp)
{
	temp[0]=(((GLfloat)(winx-tbwidth)/(GLfloat)(winw-tbwidth))-.5)*2.0*plotradius[0]+origin[0];
	temp[1]=(((GLfloat)winy/(GLfloat)winh)-.5)*2.0*plotradius[1]+origin[1];
}

bool checkrange(float num,float lower,float upper)
{
	if(num<=upper&&lower<=num)return true;
	else return false;
}

class varstype
{
	public:
	vector<coordtype> point;
	vector<vecttype> accel;
	vector<vecttype> veloc;
	float comotion;
	short dragpoint;
	
	short overpoint(GLint xval,GLint yval);
	
	bool inrange(float num,float lower,float upper)
	{
		if(num<=upper&&lower<num)return true;
		else return false;
	}
	void newpoint(GLfloat xval,GLfloat yval,bool recstate)
	{
		coordtype tempcoord;
		plotvala(xval,yval,tempcoord.x);
		tempcoord.recursive=recstate;
		tempcoord.free=true;
		point.push_back(tempcoord);
		
		vecttype tempvect;
		tempvect.x[0]=tempvect.x[1]=0;
		veloc.push_back(tempvect);
		accel.push_back(tempvect);
	}
	bool deletepoint(short pnum)
	{
		if(pnum>-1)
		{
			point.erase(point.begin()+pnum);
			veloc.erase(veloc.begin()+pnum);
			accel.erase(accel.begin()+pnum);
			return true;
		}
		else return false;	
	}
	short checkitnum();
	void ctorgb(GLfloat value);
	void pointmotion();
};
varstype vars;

void setarray3(GLfloat *vect,GLfloat r,GLfloat g,GLfloat b)
{
	vect[0]=r;
	vect[1]=g;
	vect[2]=b;
}

void depthbox(GLint x,GLint y,GLint xright,GLint ylower,GLfloat r,GLfloat g,GLfloat b,GLfloat contrast,short depth)//contrast must be between 0 and 1
{
	glColor3f(r,g,b);
	glRecti(x,y,xright,ylower);
	if(depth==1)glColor3f(.3,.3,.3);
	else if(depth==-1)glColor3f(.9,.9,.9);
	else glColor3f(.1,.1,.1);
	glBegin(GL_LINE_STRIP);
		glVertex2i(xright,y);
		glVertex2i(xright,ylower);
		glVertex2i(x,ylower);
	glEnd();
	if(depth==1)glColor3f(.9,.9,.9);
	else if(depth==-1)glColor3f(.3,.3,.3);
	else glColor3f(.1,.1,.1);
	glBegin(GL_LINE_STRIP);
		glVertex2i(x,ylower);
		glVertex2i(x,y);
		glVertex2i(xright,y);
	glEnd();
}

short wordlen(string text,short curloc)
{
	short i,len=glutBitmapWidth(STD_FONT2,text[curloc]);
	for(i=curloc+1;i<text.size();i++)
	{
		if(text[i]!=' '&&text[i]!='\n'&&text[i]!='\0')
			len+=glutBitmapWidth(STD_FONT2,text[i]);
		else break;
	}
	return len;
}

void drawparagraph(GLint x,GLint xend,GLint y,GLint yend,string text,GLfloat r,GLfloat g,GLfloat b)
{
	short int i;
	if(x>xend)
	{
		i=x;
		x=xend;
		xend=i;
	}
	if(y>yend)
	{
		i=y;
		y=yend;
		yend=i;
	}
	short curloc[2]={x,y};

	glColor3f(r,g,b);
	glRasterPos2i(x,y);
	for(i=0;i<text.size();i++)
	{
		if(text[i]!='\n'&&(text[i]==' '||curloc[0]+wordlen(text,i)<xend||wordlen(text,i)>=xend-x))
		{
			curloc[0]+=glutBitmapWidth(STD_FONT2,text[i]);
			glutBitmapCharacter(STD_FONT2,text[i]);
		}
		else
		{
			curloc[1]+=STD_FONT2_HEIGHT;
			if(curloc[1]<yend)
			{
				glRasterPos2i(x,curloc[1]);
				curloc[0]=x;
			}
			else break;
			glutBitmapCharacter(STD_FONT2,text[i]);
		}
	}
}

class helptype
{
	GLint x,y;
	string text;
	
	public:
	void set(GLint a,GLint b,string h)
	{
		helpmode=2;
		x=a;
		y=b;
		text=h;
	}
	void disp()
	{
		short direc[2],i;
		GLint h=0;
	
		if(x<winw/2)direc[0]=1;
		else direc[0]=-1;
		if(y<winh/2)direc[1]=1;
		else direc[1]=-1;
	
	
		for(i=0;i<text.size();i++)
			h+=glutBitmapWidth(GLUT_BITMAP_HELVETICA_12,text[i]);
		h=STD_FONT2_HEIGHT*(h/260)+1;
	
		depthbox(x,y,x+direc[0]*300,y+direc[1]*(35+h),1,1,.7,.5,1);
		drawparagraph(x+direc[0]*20,x+direc[0]*280,y+direc[1]*20,y+direc[1]*(20+h),text,0,0,0);
		helpmode=1;
	}
}helpvars;

class toggletype
{
	vector<string> text;
	string help;
	GLfloat color[3],hovercolor[3],clickcolor[3];
	bool direc;//0==horizontal,1==vertical
	
	public:
	GLint x,xend,y,yend,type;
	short enabled;
	void set(GLint a,GLint b,GLint c,GLint d,bool dir,vector<string> str,string h)
	{
		x=a;
		xend=b;
		y=c;
		yend=d;
		direc=dir;
		text=str;
		help=h;
		
		setarray3(color,.7,.7,.7);
		setarray3(hovercolor,.75,.75,.75);
		setarray3(clickcolor,.9,.9,.9);
		enabled=1;
	}
	short inrange(GLint locx,GLint locy)
	{
		int i;
		if(direc==false)
		{
			for(i=0;i<text.size();i++)
				if(x+(xend-x)*i<locx&&locx<xend+(xend-x)*i&&y<locy&&locy<yend)return i+1;
		}
		else
		{
			for(i=0;i<text.size();i++)
				if(x<locx&&locx<xend&&y+(yend-y)*i<locy&&locy<yend+(yend-y)*i)return i+1;
		}
		return 0;
	}
	void disp()
	{
		int i;
		
		if(direc)
			for(i=0;i<text.size();i++)
			{
				if(i+1==enabled)
					depthbox(x,y+(yend-y)*i,xend,yend+(yend-y)*i,clickcolor[0],clickcolor[1],clickcolor[2],.5,-1);	
				else
				{
					if(inrange(mouseloc[0],mouseloc[1]))
						depthbox(x,y+(yend-y)*i,xend,yend+(yend-y)*i,hovercolor[0],hovercolor[1],hovercolor[2],.5,1);
					else depthbox(x,y+(yend-y)*i,xend,yend+(yend-y)*i,color[0],color[1],color[2],.5,1);
				}
			}
		else
			for(i=0;i<text.size();i++)
			{
				if(i+1==enabled)
					depthbox(x+(xend-x)*i,y,xend+(xend-x)*i,yend,clickcolor[0],clickcolor[1],clickcolor[2],.5,-1);	
				else
				{
					if(inrange(mouseloc[0],mouseloc[1]))
						depthbox(x+(xend-x)*i,y,xend+(xend-x)*i,yend,hovercolor[0],hovercolor[1],hovercolor[2],.5,1);
					else depthbox(x+(xend-x)*i,y,xend+(xend-x)*i,yend,color[0],color[1],color[2],.5,1);
				}
			}
		
		
		glColor3f(0,0,0);
		
		int n;
		for(n=0;n<text.size();n++)
		{
			if(direc)glRasterPos2i(x+textoffset[0],yend+(yend-y)*n-textoffset[1]);
			else glRasterPos2i(x+(xend-x)*n+textoffset[0],yend-textoffset[1]);
			for(i=0;i<text[n].size();i++)
				glutBitmapCharacter(STD_FONT,text[n][i]);
		}
		if(helpmode)
			if(inrange(mouseloc[0],mouseloc[1]))
				helpvars.set(mouseloc[0],mouseloc[1],help);
	}
	short click(GLint locx,GLint locy)
	{
		int result=inrange(locx,locy);
		if(result>0)enabled=result;
		return result;
	}
};
	

class buttontype
{
	string text,help;
	GLfloat color[3],hovercolor[3],clickcolor[3];
	
	public:
	GLint x,xend,y,yend,type;
	bool enabled;
	void set(GLint a,GLint b,GLint c,GLint d,string str,string h)
	{
		x=a;
		xend=b;
		y=c;
		yend=d;
		text=str;
		help=h;
		
		setarray3(color,.7,.7,.7);
		setarray3(hovercolor,.75,.75,.75);
		setarray3(clickcolor,.9,.9,.9);
	}
	void reset()
	{
		enabled=0;
	}
	bool inrange(GLint locx,GLint locy)
	{
		if(x<locx&&locx<xend&&y<locy&&locy<yend) return true;
		else return false;
	}
	void disp()
	{
		int i;
		
		if(enabled)
			depthbox(x,y,xend,yend,clickcolor[0],clickcolor[1],clickcolor[2],.5,-1);
		else
		{
			if(inrange(mouseloc[0],mouseloc[1]))
				depthbox(x,y,xend,yend,hovercolor[0],hovercolor[1],hovercolor[2],.5,1);
			else depthbox(x,y,xend,yend,color[0],color[1],color[2],.5,1);
		}
		
		
		glColor3f(0,0,0);
		glRasterPos2i(x+textoffset[0],yend-textoffset[1]);
		for(i=0;i<text.size();i++)
			glutBitmapCharacter(STD_FONT,text[i]);
		if(helpmode)
			if(inrange(mouseloc[0],mouseloc[1]))
				helpvars.set(mouseloc[0],mouseloc[1],help);
	}
	bool click(GLint locx,GLint locy)
	{
		if(inrange(locx,locy))
		{
			enabled=(enabled+1)%2;
			return 1;
		}
		else return 0;
	}
};

GLint dispstring(int x,int y,string text)
{
	int n;
	
	glRasterPos2i(x,y);
	for(n=0;n<text.size();n++)
		glutBitmapCharacter(STD_FONT,text[n]);
}

class fieldtype
{
	short cursorplace,fieldtype;//type1==numeric, type2==alphanumeric
	GLint cursorloc;
	string help;
	
	public:
	string text;
	GLint x,xend,y,yend;
	bool enabled;
	void set(GLint a,GLint b,GLint c,GLint d,short t,string h)
	{
		x=a;
		y=b;
		xend=c;
		yend=d;
		fieldtype=t;
		enabled=false;
		help=h;
	}
	void shiftcursor(int key)
	{
		if(key==GLUT_KEY_LEFT&&cursorplace>0)
		{
			cursorplace--;
			cursorloc-=glutBitmapWidth(STD_FONT,text[cursorplace]);
		}
		else if(key==GLUT_KEY_RIGHT&&cursorplace<text.size())
		{
			cursorloc+=glutBitmapWidth(STD_FONT,text[cursorplace]);
			cursorplace++;
		}
	}
	short textsize()
	{
		int i,size=0;
		for(i=0;i<text.size();i++)
			size+=glutBitmapWidth(STD_FONT,text[i]);
		return size;
	}
	bool inrange(GLint locx,GLint locy)
	{
		if(x<locx&&locx<xend&&y<locy&&locy<yend) return true;
		else return false;
	}
	void disp()
	{
		depthbox(x,y,xend,yend,1,1,1,.5,-1);
		glColor3f(0,0,0);
		dispstring(x+textoffset[0],yend-textoffset[1],text);
		if(helpmode)
		{
			if(inrange(mouseloc[0],mouseloc[1]))
				helpvars.set(mouseloc[0],mouseloc[1],help);
		}
		else if(clock()%800000<600000)
		{
			glBegin(GL_LINES);
				glVertex2f(cursorloc+x,y+2);
				glVertex2f(cursorloc+x,yend-2);
			glEnd();
		}
	}
	
	int click(int xloc,int yloc)
	{
		int n,m;
		double placestart,placeend;
		if(inrange(xloc,yloc))
		{
			xloc-=x;
			cursorplace=text.size();
			cursorloc=0;
			placestart=0;
			placeend=textoffset[0]+(glutBitmapWidth(STD_FONT,text[0])/2.0);
	
			for(m=0;m<cursorplace;m++,placestart=placeend,placeend+=(glutBitmapWidth(STD_FONT,text[m-1])/2.0)+(glutBitmapWidth(STD_FONT,text[m])/2.0))
				if(placestart<xloc&&xloc<=placeend)
				{
					cursorplace=m;
					cursorloc=placeend-glutBitmapWidth(STD_FONT,text[m])/2;
					break;
				}
			if(cursorloc==0)
				cursorloc=placeend;
			enabled=true;
			return n;
		}
		else
		{
			enabled=false;
			return -1;
		}
	}

	bool type(char a)
	{
		if(enabled)
		{
			if(fieldtype==1)
			{
				if(checkrange(a,48,57)||a==45||a==46)
				{
					if(textsize()+glutBitmapWidth(STD_FONT,a)<xend-x-2*textoffset[0])
					{
						text.insert(cursorplace,1,a);
						cursorplace++;
						cursorloc+=glutBitmapWidth(STD_FONT,a);
					}
					return true;
				}
			}
			else if(checkrange(a,48,57)||checkrange(a,65,90)||checkrange(a,97,122))
			{
				if(textsize()+glutBitmapWidth(STD_FONT,a)<xend-x-2*textoffset[0]&&(((int)a>=48&&(int)a<=122)||a==' '))
				{
					text.insert(cursorplace,1,a);
					cursorplace++;
					cursorloc+=glutBitmapWidth(STD_FONT,a);
				}
				return true;
			}
			if(a==13)
			{
				enabled=false;
				return false;
			}
			if(cursorplace>0&&a==8)
			{
				cursorloc-=glutBitmapWidth(STD_FONT,text[cursorplace-1]);
				text.erase(cursorplace-1,1);
				cursorplace--;
				return true;
			}
			
		}
		else return true;
	}
};

class incrementboxtype
{
	GLint arrowwidth,boxwidth,height;
	float min,max,increment;
	string text,help;
	
	public:
	GLint textx,x,xend,y,yend;
	short numtype;
	float val;
	fieldtype field;
	void set(GLint a,GLint b,GLint c,GLint d,GLint e,float f,float mi,float ma,short nt,float startval,string g,string h)
	{
		textx=x=a;
		y=b;
		arrowwidth=c;
		boxwidth=d;
		height=e;
		increment=f;
		min=mi;
		max=ma;
		numtype=nt;
		val=startval;
		text=g;
		help=h;
		
		recalcx();
	}
	void recalcx()
	{
		int n;
		char tempchar[10];
		for(n=0;n<text.size();n++)
			textx-=glutBitmapWidth(STD_FONT,text[n]);
		textx-=5;
		xend=x+arrowwidth*2+4+boxwidth;
		yend=y+height;
		
		field.set(x+arrowwidth+2,y,x+arrowwidth+boxwidth,y+height,1,help);
		if(numtype)
			sprintf(tempchar,"%1.1f",val);
		else
			sprintf(tempchar,"%d",(int)val);
		field.text=tempchar;
	}
	bool inrange(GLint locx,GLint locy)
	{
		if(x<locx&&locx<xend&&y<locy&&locy<yend&&!field.inrange(locx,locy))return true;
		else return false;
	}
	void disp()
	{
		char tempchar[10];
		
		glColor3f(0,0,0);
		dispstring(textx,y+height,text);
		
		depthbox(x,y,x+arrowwidth,y+height,.7,.7,.7,.5,1);
		depthbox(x+arrowwidth+4+boxwidth,y,x+2*arrowwidth+4+boxwidth,y+height,.7,.7,.7,.5,1);
		
		field.disp();
	
		glColor3f(0,0,0);
		glBegin(GL_POLYGON);
			glVertex2i(x+arrowwidth-3,y+3);
			glVertex2i(x+3,y+(height/2));
			glVertex2i(x+arrowwidth-3,y+height-3);
		glEnd();
		glBegin(GL_POLYGON);
			glVertex2i(x+arrowwidth+4+boxwidth+3,y+3);
			glVertex2i(x+2*arrowwidth+4+boxwidth-3,y+(height/2));
			glVertex2i(x+arrowwidth+4+boxwidth+3,y+height-3);
		glEnd();
		
		if(helpmode)
			if(inrange(mouseloc[0],mouseloc[1]))
				helpvars.set(mouseloc[0],mouseloc[1],help);			
	}
	void checksize()
	{
		float tempval=val;
		if(val>max)val=max;
		else if(val<min)val=min;
	}
	bool type(char key)
	{
		char tempchar[10];
		if(field.type(key)==false)
		{
			for(int i=0;i<field.text.size(),i<10;i++)
				tempchar[i]=field.text[i];
			if(atof(tempchar)==val)return false;
			else
			{
				val=atof(tempchar);
				checksize();
				return true;
			}
		}
		else return false;
	}
	short click(GLint locx,GLint locy)
	{
		char tempchar[10];
		if(field.enabled)
		{
			if(field.click(locx,locy)==-1)
			{
				for(int i=0;i<field.text.size(),i<10;i++)
					tempchar[i]=field.text[i];
				if(atof(tempchar)==val)return 0;
				else
				{
					val=atof(tempchar);
					checksize();
					return 1;
				}
			}
			else return 0;
		}
		else if(x<locx&&locx<x+arrowwidth&&y<locy&&locy<y+height)
		{
			val-=increment;
			checksize();
			
			if(numtype)
				sprintf(tempchar,"%1.1f",val);
			else
				sprintf(tempchar,"%d",(int)val);
			field.text=tempchar;
			return -1;
		}
		else if(x+boxwidth+arrowwidth+4<locx&&locx<x+2*arrowwidth+boxwidth+4&&y<locy&&locy<y+height)
		{
			val+=increment;
			checksize();
			
			if(numtype)
				sprintf(tempchar,"%1.1f",val);
			else
				sprintf(tempchar,"%d",(int)val);
			field.text=tempchar;
			return 1;
		}
		else
		{
			field.click(locx,locy);
			return 0;
		}
	}
};
fieldtype rendnamefield;
toggletype sttog,ftypetog;
buttontype editbut,clearbut,prevbut,evenbut,oddbut,playpbut,playcbut,renderbut,startrenderbut,lockratiobut,symbut,flipbut,deppbut,depibut,rendnamebut,freezebut,mod1but,mod2but,fsbut,helpbut,originbut,infobut;
incrementboxtype itbox,ptsizebox,cdfbox,blendbox,cobox,speedpbox,speedcbox,rpnumbox,rendwbox,rendhbox,rendibox;

short varstype::checkitnum()
{
	short toohigh=0;
	if(point.size()<=2)return 0;
	if(rpnumbox.val==1)
	{
		int i,pnum=0;
		for(i=2;i<vars.point.size();i++)
			if(vars.point[i].recursive==true)pnum++;
		while((point.size()-1-pnum)*pow(pnum*(1+evenbut.enabled)*(1+oddbut.enabled),itbox.val)>pow((float)2,15))
		{
			toohigh=1;
			itbox.val--;
		}
	}
	else if(rpnumbox.val==2)
	{
		int i,pnum=0;
		for(i=2;i<vars.point.size();i++)
			if(vars.point[i].recursive==true)pnum++;
		while((point.size()-1-pnum)*pow(pnum*(1+evenbut.enabled)*(1+oddbut.enabled)+1,itbox.val)>pow((float)2,15))
		{
			toohigh=1;
			itbox.val--;
		}
	}
	if(toohigh)
	{
		char tempchar[10];
		sprintf(tempchar,"%d",(int)itbox.val);
		itbox.field.text=tempchar;
	}
	return toohigh;
}
short varstype::overpoint(GLint x,GLint y)
{
	short i;
	coordtype p;
	plotvala(x,y,p.x);
	for(i=0;i<point.size();i++)
		if(sqrt(pow(p.x[0]-point[i].x[0],2)+pow(p.x[1]-point[i].x[1],2))<=(GLfloat)ptsizebox.val*plotradius[0]*10.0/(GLfloat)winw)
			return i;
	return -1;
}
void varstype::ctorgb(GLfloat value) //converts value to RGB color
{
	value=value*cdfbox.val+cobox.val; //squashes colors according to cdfbox.val    
	while(value>=3.0)
		value-=3.0;
	while(value<0)
		value+=3.0;		
		
	if(value<0.5||value>=2.5)ctemp[0]=1;
	else if(inrange(value,.5,1))ctemp[0]=2*(1.0-value);
	else if(inrange(value,2,2.5))ctemp[0]=2*(value-2);
	else ctemp[0]=0;
	
	if(inrange(value,.5,1.5))ctemp[1]=1;
	else if(inrange(value,0,.5))ctemp[1]=2.0*value;
	else if(inrange(value,1.5,2))ctemp[1]=2.0*(2.0-value);
	else ctemp[1]=0;

	if(inrange(value,1.5,2.5))ctemp[2]=1;
	else if(inrange(value,2.5,3))ctemp[2]=2.0*(3.0-value);
	else if(inrange(value,1,1.5))ctemp[2]=2.0*(value-1.0);
	else ctemp[2]=0;
	
	ctemp[3]=pow(2,-blendbox.val);
}

void varstype::pointmotion()
{
	int i;
	for(i=0;i<point.size();i++)
	{
		if(point[i].free)
		{
			if(rand()%300==1)
			{
				accel[i].x[0]=((rand()%11)-5)/5.0;
				accel[i].x[1]=((rand()%11)-5)/5.0;
			}
			accel[i].x[0]-=point[i].x[0]*.1/100.0;
			accel[i].x[1]-=point[i].x[1]*.1/100.0;
		
			if(veloc[i].x[0]<speedpbox.val)veloc[i].x[0]=(.99*veloc[i].x[0])+(.05*accel[i].x[0]);
			else veloc[i].x[0]--;
			if(veloc[i].x[1]<speedpbox.val)veloc[i].x[1]=(.99*veloc[i].x[1])+(.05*accel[i].x[1]);
			else veloc[i].x[1]--;
		
			point[i].x[0]+=veloc[i].x[0]/50.0;
			point[i].x[1]+=veloc[i].x[1]/50.0;
		}
	}
}

void pushback(vector<coordtype> *v,GLfloat xval,GLfloat yval)
{
	coordtype temp;
	temp.x[0]=xval;
	temp.x[1]=yval;
	v->push_back(temp);
}
vector<vecttype> ratio;

struct rgbtype
{
	short r;
	short g;
	short b;
};

short tempx[2],tempp[2];
vector<vector<rgbtype> > bitmap;
void recursion2save(GLfloat a0,GLfloat a1,GLfloat b0,GLfloat b1,GLfloat curcolor,short direc,short i,short ncur)
{
	if(mod2but.enabled==false||rpnumbox.val==1)ncur=2;
	for(;ncur<vars.point.size();ncur++)
	{
		if(ncur>2&&i>1&&deppbut.enabled)curcolor+=(cdfbox.val/((GLfloat)(i-1)));
		vars.ctorgb(curcolor+vars.comotion);
		
		tempx[1]=(int)(((((b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1)-renderarea[0][1])*rendhbox.val)/(renderarea[1][1]-renderarea[0][1]));
		tempx[0]=(int)(((((b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0)-renderarea[0][0])*rendwbox.val)/(renderarea[1][0]-renderarea[0][0]));
		for(tempp[0]=0;tempp[0]<ptsizebox.val;tempp[0]++)
		{
			if(tempx[0]+tempp[0]>=rendwbox.val||tempx[0]+tempp[0]<0)break;
			for(tempp[1]=0;tempp[1]<ptsizebox.val;tempp[1]++)
			{
				if(tempx[1]+tempp[1]>=rendhbox.val||tempx[1]+tempp[1]<0)break;
				bitmap[tempx[1]+tempp[1]][tempx[0]+tempp[0]].r=ctemp[3]*ctemp[0]*256+bitmap[tempx[1]+tempp[1]][tempx[0]+tempp[0]].r*(1.0-ctemp[3]);
				bitmap[tempx[1]+tempp[1]][tempx[0]+tempp[0]].g=ctemp[3]*ctemp[1]*256+bitmap[tempx[1]+tempp[1]][tempx[0]+tempp[0]].g*(1.0-ctemp[3]);
				bitmap[tempx[1]+tempp[1]][tempx[0]+tempp[0]].b=ctemp[3]*ctemp[2]*256+bitmap[tempx[1]+tempp[1]][tempx[0]+tempp[0]].b*(1.0-ctemp[3]);
			}
		}
		
		if(i<rendibox.val&&vars.point[ncur].recursive)
		{
			if(rpnumbox.val==2)
			{
				recursion2save(a0,a1,(b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0,(b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1,curcolor-(depibut.enabled*cdfbox.val/(GLfloat)i),direc-(2*direc*flipbut.enabled),i+1,ncur);
				if(symbut.enabled)recursion2save(b0,b1,(b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0,(b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1,curcolor+(depibut.enabled*cdfbox.val/(GLfloat)i),-direc+(2*direc*flipbut.enabled),i+1,ncur);
				else recursion2save((b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0,(b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1,b0,b1,curcolor+(depibut.enabled*cdfbox.val/(GLfloat)i),direc-(2*direc*flipbut.enabled),i+1,ncur);
			}
			else recursion2save(b0,b1,(b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0,(b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1,curcolor+(depibut.enabled*cdfbox.val/(GLfloat)i),direc-(2*direc*flipbut.enabled),i+1,ncur);
			if(oddbut.enabled)
			{
				if(rpnumbox.val==2)
				{
					recursion2save((b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0,(b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1,a0,a1,curcolor+(depibut.enabled*cdfbox.val/(GLfloat)i),direc-(2*direc*flipbut.enabled),i+1,ncur);
					if(mod1but.enabled==false)recursion2save(b0,b1,(b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0,(b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1,curcolor-(depibut.enabled*cdfbox.val/(GLfloat)i),direc-(2*direc*flipbut.enabled),i+1,ncur);
				}
				else recursion2save(b0,b1,(b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0,(b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1,curcolor-(depibut.enabled*cdfbox.val/(GLfloat)i),direc-(2*direc*flipbut.enabled),i+1,ncur);
					
			}
			if(evenbut.enabled)
			{
				if(rpnumbox.val==2)
				{
					recursion2save(a0,a1,(b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0,(b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1,curcolor-(depibut.enabled*cdfbox.val/(GLfloat)i),-direc+(2*direc*flipbut.enabled),i+1,ncur);
					recursion2save((b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0,(b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1,b0,b1,curcolor+(depibut.enabled*cdfbox.val/(GLfloat)i),-direc+(2*direc*flipbut.enabled),i+1,ncur);
				}
				else recursion2save(b0,b1,(b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0,(b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1,curcolor+(depibut.enabled*cdfbox.val/(GLfloat)i),-direc+(2*direc*flipbut.enabled),i+1,ncur);
			}
		}
	}
}

float renderimage()
{
	int x,y;
	GLfloat tempf;
	rgbtype black={0,0,0};
	vector<rgbtype> blackarray(rendwbox.val,black);
	for(y=0;y<rendhbox.val;y++)
		bitmap.push_back(blackarray);
	plotvala(renderarea[0][0],renderarea[0][1],renderarea[0]);
	plotvala(renderarea[1][0],renderarea[1][1],renderarea[1]);

	for(y=0;y<rendhbox.val;y++)
		for(x=0;x<rendwbox.val;x++)
			bitmap[y][x].r=bitmap[y][x].g=bitmap[y][x].b=0;
			
	recursion2save(vars.point[0].x[0],vars.point[0].x[1],vars.point[1].x[0],vars.point[1].x[1],0,1,1,2);
	if(oddbut.enabled)
		recursion2save(vars.point[1].x[0],vars.point[1].x[1],vars.point[0].x[0],vars.point[0].x[1],0,1,1,2);
	else if(evenbut.enabled)
		recursion2save(vars.point[0].x[0],vars.point[0].x[1],vars.point[1].x[0],vars.point[1].x[1],0,-1,1,2);
	
	char *name=new char[rendnamefield.text.size()+5];
	ofstream ofile;
	BMP bmpimage;
	switch(ftypetog.enabled)
	{
		case 1:
			SetEasyBMPwarningsOff();
			bmpimage.SetBitDepth(256);
			bmpimage.SetSize((int)rendwbox.val,(int)rendhbox.val);
			for(y=0;y<rendhbox.val;y++)
				for(x=0;x<rendwbox.val;x++)
				{
					bmpimage(x,y)->Red=bitmap[y][x].r;
					bmpimage(x,y)->Green=bitmap[y][x].g;
					bmpimage(x,y)->Blue=bitmap[y][x].b;
				}
			
			rendnamefield.text.append(".bmp");
			name=(char*)rendnamefield.text.c_str();
			bmpimage.WriteToFile(name);
			break;
		case 2:
			rendnamefield.text.append(".ppm");
			name=(char*)rendnamefield.text.c_str();
			ofile.open(name,ios::trunc);
			
			ofile<<"P3 "<<rendwbox.val<<" "<<rendhbox.val<<" 256";
			for(y=0;y<rendhbox.val;y++)
				for(x=0;x<rendwbox.val;x++)
					ofile<<" "<<bitmap[y][x].r<<" "<<bitmap[y][x].g<<" "<<bitmap[y][x].b;
			break;
	}	
	ofile.close();
	bitmap.clear();
	renderprogress=1;
	
	renderprogress=0;
	renderarea[0][0]=renderarea[0][1]=renderarea[1][0]=renderarea[1][1]=-1;
	renderareaselect=false;
	startrenderbut.enabled=0;
	renderbut.enabled=0;
}			

void recursion2(GLfloat a0,GLfloat a1,GLfloat b0,GLfloat b1,GLfloat curcolor,short direc,short i,short ncur)
{
	if(mod2but.enabled==false||rpnumbox.val==1)ncur=2;
	for(;ncur<vars.point.size();ncur++)
	{
		glBegin(GL_POINTS);
			if(ncur>2&&i>1&&deppbut.enabled)curcolor+=(cdfbox.val/((GLfloat)(i-1)));
			vars.ctorgb(curcolor+vars.comotion);
			if(editbut.enabled)ctemp[3]=1;
			glColor4fv(ctemp);
			glVertex2f((b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0,(b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1);
		glEnd();
		
		if(i<itbox.val-(float)((itbox.val)*editbut.enabled)/3.0&&vars.point[ncur].recursive)
		{
			if(rpnumbox.val==2)
			{
				recursion2(a0,a1,(b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0,(b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1,curcolor-(depibut.enabled*cdfbox.val/(GLfloat)i),direc-(2*direc*flipbut.enabled),i+1,ncur);
				if(symbut.enabled)recursion2(b0,b1,(b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0,(b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1,curcolor+(depibut.enabled*cdfbox.val/(GLfloat)i),-direc+(2*direc*flipbut.enabled),i+1,ncur);
				else recursion2((b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0,(b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1,b0,b1,curcolor+(depibut.enabled*cdfbox.val/(GLfloat)i),direc-(2*direc*flipbut.enabled),i+1,ncur);
			}
			else recursion2(b0,b1,(b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0,(b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1,curcolor+(depibut.enabled*cdfbox.val/(GLfloat)i),direc-(2*direc*flipbut.enabled),i+1,ncur);
			if(oddbut.enabled)
			{
				if(rpnumbox.val==2)
				{
					recursion2((b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0,(b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1,a0,a1,curcolor+(depibut.enabled*cdfbox.val/(GLfloat)i),direc-(2*direc*flipbut.enabled),i+1,ncur);
					if(mod1but.enabled==false)recursion2(b0,b1,(b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0,(b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1,curcolor-(depibut.enabled*cdfbox.val/(GLfloat)i),direc-(2*direc*flipbut.enabled),i+1,ncur);
				}
				else recursion2(b0,b1,(b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0,(b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1,curcolor-(depibut.enabled*cdfbox.val/(GLfloat)i),direc-(2*direc*flipbut.enabled),i+1,ncur);
					
			}
			if(evenbut.enabled)
			{
				if(rpnumbox.val==2)
				{
					recursion2(a0,a1,(b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0,(b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1,curcolor-(depibut.enabled*cdfbox.val/(GLfloat)i),-direc+(2*direc*flipbut.enabled),i+1,ncur);
					recursion2((b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0,(b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1,b0,b1,curcolor+(depibut.enabled*cdfbox.val/(GLfloat)i),-direc+(2*direc*flipbut.enabled),i+1,ncur);
				}
				else recursion2(b0,b1,(b0-a0)*ratio[ncur-2].x[0]+direc*(a1-b1)*ratio[ncur-2].x[1]+a0,(b1-a1)*ratio[ncur-2].x[0]+direc*(b0-a0)*ratio[ncur-2].x[1]+a1,curcolor+(depibut.enabled*cdfbox.val/(GLfloat)i),-direc+(2*direc*flipbut.enabled),i+1,ncur);
			}
		}
	}
}

float distv(float a[2],float b[2])
{
	return sqrt(pow(b[0]-a[0],2)+pow(b[1]-a[1],2));
}

void alphabeta(GLfloat *x1,GLfloat *x2,GLfloat *x3,vecttype *target)
{
	GLfloat m[2],theta;
	if(x2[0]-x1[0]==0)m[0]=999999;
	else m[0]=(x2[1]-x1[1])/(x2[0]-x1[0]);
	if(x3[0]-x1[0]==0)m[1]=999999;
	else m[1]=(x3[1]-x1[1])/(x3[0]-x1[0]);
	
	theta=atan(m[1])-atan(m[0]);
	target->x[0]=distv(x1,x3)*cos(theta)/distv(x1,x2);
	target->x[1]=distv(x1,x3)*sin(theta)/distv(x1,x2);
	if(x2[0]<x1[0])
	{
		target->x[1]=-target->x[1];
		target->x[0]=-target->x[0];
	}
	if(x3[0]<x1[0])
	{
		target->x[1]=-target->x[1];
		target->x[0]=-target->x[0];
	}
}

void render(short num)
{
	int i,selpoint=vars.overpoint(mouseloc[0],mouseloc[1]);
	vecttype temp;
	
	switch(num)
	{
		case 1:
			toberend=0;
			glNewList(num,GL_COMPILE);
				if(editbut.enabled==0||prevbut.enabled)
				{
					glMatrixMode(GL_PROJECTION);
					glLoadIdentity();
					glMatrixMode(GL_MODELVIEW);
					glLoadIdentity();
					if(fsbut.enabled)
					{
						glViewport(0,0,winw,winh);
						gluOrtho2D((-plotradius[0]+origin[0])*winw/(winw-tbwidth),(plotradius[0]+origin[0])*winw/(winw-tbwidth),plotradius[1]+origin[1],-plotradius[1]+origin[1]);
					}
					else
					{
						glViewport(tbwidth,0,winw-tbwidth,winh);
						gluOrtho2D(-plotradius[0]+origin[0],plotradius[0]+origin[0],plotradius[1]+origin[1],-plotradius[1]+origin[1]);
					}
				
					ratio.clear();
					if(vars.point.size()>2)
						for(i=2;i<vars.point.size();i++)
						{
							alphabeta(vars.point[0].x,vars.point[1].x,vars.point[i].x,&temp);
							ratio.push_back(temp);
						}
					glPointSize(ptsizebox.val+editbut.enabled*3);
					vars.ctorgb(0);
					glColor4fv(ctemp);
				
					if(vars.point.size()>2)
					{
						glBegin(GL_POINTS);
							if(rpnumbox.val!=1||oddbut.enabled)glVertex2fv(vars.point[0].x);
							glVertex2fv(vars.point[1].x);
						glEnd();
						recursion2(vars.point[0].x[0],vars.point[0].x[1],vars.point[1].x[0],vars.point[1].x[1],0,1,1,2);
						if(oddbut.enabled)
							recursion2(vars.point[1].x[0],vars.point[1].x[1],vars.point[0].x[0],vars.point[0].x[1],0,1,1,2);
						else if(evenbut.enabled&&rpnumbox.val>1)
							recursion2(vars.point[0].x[0],vars.point[0].x[1],vars.point[1].x[0],vars.point[1].x[1],0,-1,1,2);
					}
				}
			glEndList();
			break;
		case 2:
			glNewList(num,GL_COMPILE);
				if(editbut.enabled)
				{
					glMatrixMode(GL_PROJECTION);
					glLoadIdentity();
					glMatrixMode(GL_MODELVIEW);
					glLoadIdentity();
					if(fsbut.enabled)
					{
						glViewport(0,0,winw,winh);
						gluOrtho2D((-plotradius[0]+origin[0])*winw/(winw-tbwidth),(plotradius[0]+origin[0])*winw/(winw-tbwidth),plotradius[1]+origin[1],-plotradius[1]+origin[1]);
					}
					else
					{
						glViewport(tbwidth,0,winw-tbwidth,winh);
						gluOrtho2D(-plotradius[0]+origin[0],plotradius[0]+origin[0],plotradius[1]+origin[1],-plotradius[1]+origin[1]);
					}
				
					glPointSize(5+ptsizebox.val+editbut.enabled*10);
						for(i=0;i<vars.point.size()&&i<2;i++)
						{
							if(!vars.point[i].free)
							{
								glPointSize(13+ptsizebox.val+editbut.enabled*10);
								glColor3f(.2,.2,1);
								glBegin(GL_POINTS);
									glVertex2fv(vars.point[i].x);
								glEnd();
								glPointSize(5+ptsizebox.val+editbut.enabled*10);
							}
							if(i==selpoint)glColor3f(1,0,0);
							else if(vars.point[i].recursive)glColor3f(1,1,1);
							else glColor3f(1,1,0);
							glBegin(GL_POINTS);
								glVertex2fv(vars.point[i].x);
							glEnd();
						}
					
					glPointSize(ptsizebox.val+editbut.enabled*10);
						for(i=2;i<vars.point.size();i++)
						{
							if(!vars.point[i].free)
							{
								glPointSize(8+ptsizebox.val+editbut.enabled*10);
								glColor3f(.2,.2,1);
								glBegin(GL_POINTS);
									glVertex2fv(vars.point[i].x);
								glEnd();
								glPointSize(ptsizebox.val+editbut.enabled*10);
							}
							if(i==selpoint)glColor3f(1,0,0);
							else if(vars.point[i].recursive)glColor3f(1,1,1);
							else glColor3f(1,1,0);
							glBegin(GL_POINTS);
								glVertex2fv(vars.point[i].x);
							glEnd();
						}
					if(selpoint>-1&&helpmode)helpvars.set(mouseloc[0],mouseloc[1],"Left click to drag a point or create a new point.  Right click to delete a point.  Editable points are visible only in Edit mode.  While in Edit mode, reference points are displayed larger than pattern points.");
					if(helpmode==2)helpvars.disp();
					if(originbut.enabled)
					{
						glBegin(GL_LINES);
							glColor3f(1,1,1);
							glVertex2f(0,5);
							glVertex2f(0,-5);
							glVertex2f(-5,0);
							glVertex2f(5,0);
						glEnd();
					}
				}
			glEndList();		
			break;
		case 3:
			glNewList(num,GL_COMPILE);
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				glViewport(0,0,winw,winh);
				gluOrtho2D(0,winw,winh,0);
				
				if(fsbut.enabled==0)
				{
					depthbox(0,0,tbwidth,winh,.5,.5,.5,.5,1);

					helpbut.disp();
					clearbut.disp();
					editbut.disp();
					renderbut.disp();
					itbox.disp();
					rpnumbox.disp();
					ptsizebox.disp();
					prevbut.disp();
					dispstring(depibut.x,depibut.y-10,"Color Dependency");
					depibut.disp();
					deppbut.disp();
					mod1but.disp();
					mod2but.disp();
				
					dispstring(playpbut.x,playpbut.y-10,"Motion");
					playpbut.disp();
					playcbut.disp();
					dispstring(speedpbox.x,playpbut.y-10,"Speed");
					if(playpbut.enabled)speedpbox.disp();
					if(playcbut.enabled)speedcbox.disp();
					cdfbox.disp();
					cobox.disp();
					blendbox.disp();
					symbut.disp();
					flipbut.disp();
					dispstring(clearbut.x,evenbut.yend-3,"Mirror:");
					evenbut.disp();
					oddbut.disp();
					freezebut.disp();
					originbut.disp();
					
					glPointSize(10);
					glBegin(GL_POINTS);
						for(i=tbwidth;i<winw;i++)
						{
							vars.ctorgb(((float)(i-tbwidth)*3.0/(float)(winw-tbwidth))-1.5);
							glColor3fv(ctemp);
							glVertex2f(i+5,winh);
						}
					glEnd();
					if(helpbut.enabled)
					{
						infobut.disp();
						if(infobut.enabled)
						{
							glColor3f(.9,.9,.9);
							glRecti(tbwidth+20,20,tbwidth+420,420);
							drawparagraph(tbwidth+30,tbwidth+400,30,410,"\nRecursionLab 0.2.01 beta\nCreated by: Andy Barrette\n\n\nFor lists of downloadable versions, known bugs, and upcoming features, go to\nhttps://sourceforge.net/projects/recursionlab\n\n\nRecursionLab is a program that allows the user to visually explore two types of geometrical recursion.  Points can be added to the recursive plot by clicking empty space on the plot.  Existing points can be dragged with the left mouse button or deleted with the right mouse button.  The toolbar on the left consists of various features that affect the geometrical, recursive, and color properties of the resulting fractal.\n\n\nTo report bugs that are not listed at the website above, please send an email to hixidom@gmail.com including the software version, your computer specifications, and the bug symptoms.  Also, please contact me by the same email with any comments, questions, or suggestions.\n\nThank you very much for your support.\n-Andy",0,0,.5);
						}
					}					
				}
				fsbut.disp();
				if(helpmode==2)helpvars.disp();
			glEndList();
			break;
		case 4:
			glNewList(num,GL_COMPILE);
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				glViewport(0,0,winw,winh);
				gluOrtho2D(0,winw,winh,0);
				
				if(renderareaselect)
				{
					glColor3f(1,1,1);
					glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
					glRecti(renderarea[0][0],renderarea[0][1],renderarea[1][0],renderarea[1][1]);
					glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
					
					depthbox(renderscreen[0],renderscreen[1],renderscreen[2],renderscreen[3],.5,.5,.5,.5,1);
					if(startrenderbut.enabled)
					{
						/*dispstring(renderscreen[0]+10,renderscreen[3]-20,"Progress:");
						glColor3f(0,0,0);
						glPolygonMode (GL_FRONT_AND_BACK,GL_LINE);
						glRecti(120,140,renderscreen[2]+100,160);
					
						glPolygonMode (GL_FRONT_AND_BACK,GL_FILL);
						glRecti(120,140,120+(renderprogress*(renderscreen[2]-20)),160);
						renderprogress=0;*/
						renderbut.enabled=startrenderbut.enabled=0;
					}
					else
					{
						rendwbox.disp();
						rendhbox.disp();
						rendibox.disp();
						dispstring(renderscreen[0]+8,rendnamefield.yend,"File Name");
						rendnamefield.disp();
						ftypetog.disp();
						lockratiobut.disp();
						startrenderbut.disp();
						helpbut.disp();
					}
				}
				else
				{
					glColor3f(1,1,1);
					dispstring(tbwidth+50,20,"Select Area to Render");
					if(renderarea[0][0]>-1)
					{
						glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
						glRecti(renderarea[0][0],renderarea[0][1],mouseloc[0],mouseloc[1]);
						glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
					}
				}
				if(helpmode==2)helpvars.disp();
					
			glEndList();
			break;
		default:
			cout<<"ERROR:  Attempting to render non-existent list ("<<num<<")!\n";
			break;
	}
}

void resize (int w, int h)
{
	winw=w; 
	winh=h;
	plotradius[1]=plotradius[0]*h/w;
	fsbut.xend=winw;
	fsbut.x=winw-30;
	render(1);
	render(2);
	gluOrtho2D(0,winw,winh,0);
	glViewport(0,0,winw,winh);
}

void init(void)
{
	char temp[50];
	
	glClearColor(0,0,0,1);
	glShadeModel(GL_SMOOTH);
	glPolygonMode (GL_FRONT_AND_BACK,GL_FILL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	//glEnable(GL_POINT_SMOOTH);
	
	vars.newpoint(((winw-tbwidth)/2)+tbwidth-(winw-tbwidth)/4,winh*4/7,true);
	vars.newpoint(((winw-tbwidth)/2)+tbwidth+(winw-tbwidth)/4,winh*4/7,true);
	vars.newpoint((winw-tbwidth)/2+tbwidth,winh*2/5,true);
	editbut.enabled=prevbut.enabled=depibut.enabled=lockratiobut.enabled=1;
	vars.dragpoint=-1;
	srand(time(NULL));
	vector<string> temptext;
	
	buttontype tempbut;
	helpbut.set(10,30,15,35,"?","Hover over a tool to learn about it's function.");
	infobut.set(helpbut.xend+20,helpbut.xend+20+60,helpbut.y,helpbut.yend,"INFO","Click for additional help and info about the author.");
	
	clearbut.set(10,90,helpbut.yend+10,helpbut.yend+30,"CLEAR","Deletes all points");
	renderbut.set(clearbut.xend+20,clearbut.xend+90,clearbut.y,clearbut.yend,"Render","Lets you crop and export the current view to a BMP or PPM image.");
	prevbut.set(clearbut.x,clearbut.x+75,clearbut.yend+10,clearbut.yend+30,"Preview","Displays a low-iteration preview while in edit mode.");
	editbut.set(prevbut.xend+20,prevbut.xend+70,prevbut.y,prevbut.yend,"EDIT","Toggles in and out of Edit mode.  Edit mode allows you to edit points.");
	
	itbox.set(100,prevbut.yend+45,20,45,20,1,1,100,0,2,"Iterations","Changes the number of iterations of the recursion algorithm that are seen when not in Edit/Preview mode.  To prevent long wait periods, the number of iterations allowed is limited.");
	rpnumbox.set(100,itbox.yend+10,20,45,20,1,1,2,0,2,"Ref.Points","Switches between two recursion modes.  1. One reference point is used.  The pattern is repeated for each pattern point. 2. Two reference points are used.  The pattern is repeated for each pattern point and each reference points.  A higher number of reference points may be implemented in a future release.");
	evenbut.set(clearbut.x+70,clearbut.x+123,rpnumbox.yend+10,rpnumbox.yend+30,"Even","Pattern points are reflected about the line connecting the two reference points.");
	oddbut.set(evenbut.xend+10,evenbut.xend+58,evenbut.y,evenbut.yend,"Odd","Pattern points are reflected about the origin at the center of the line connecting the two reference points.");
	symbut.set(clearbut.x,clearbut.x+90,evenbut.yend+10,evenbut.yend+30,"Symmetry","For 2 reference points, Symmetry mirrors recursions using the second reference point along the line connecting the two reference points.");
	flipbut.set(symbut.xend+10,symbut.xend+10+80,symbut.y,symbut.yend,"Flip-flop","Reverses the direction of recursion at each iteration");
	mod1but.set(clearbut.x,clearbut.x+55,symbut.yend+10,symbut.yend+30,"mod1","This modification effects the recursion algorithm only when Odd mirroring is enabled.");
	mod2but.set(mod1but.xend+10,mod1but.xend+65,symbut.yend+10,symbut.yend+30,"mod2","This modification effects the recursion algorithm only when more than 1 pattern point has been placed.");
	
	cdfbox.set(100,mod1but.yend+45,20,45,20,.1,0,20,1,.5,"C.Density","Changes color density, which is the rate of change of color per change in variables on which color is dependent.");
	cobox.set(100,cdfbox.yend+10,20,45,20,.1,0,27,1,0,"C.Offset","Offsets color about the variables on which color is dependent.");
	ptsizebox.set(100,cobox.yend+10,20,45,20,1,1,100,0,1,"Pt.size","Changes the size of points as they appear on your screen.");
	blendbox.set(100,ptsizebox.yend+10,20,45,20,1,0,8,0,0,"Blending","Exponentially increases the transparency of displayed points.  0 = 100% opaque, 1=50% opaque, 2=25% opaque...");
	depibut.set(clearbut.x,clearbut.x+80,blendbox.yend+45,blendbox.yend+65,"Iteration","Causes color to vary with respect to iteration.");
	deppbut.set(depibut.xend+10,depibut.xend+95,depibut.y,depibut.yend,"Pat.Point","Causes color to vary with respect to pattern point");
	
	playpbut.set(clearbut.x,clearbut.x+55,depibut.yend+85,depibut.yend+105,"Point","Causes all unfrozen points to move about freely.");
	speedpbox.set(100,playpbut.y,20,45,20,1,0,100,0,10,"","Changes the speed of point motion.");
	playcbut.set(clearbut.x,clearbut.x+60,playpbut.yend+10,playpbut.yend+30,"Color","Causes color to rotate through the color spectrum for all points regardless of frozen/free status.");
	speedcbox.set(100,playcbut.y,20,45,20,1,0,100,0,10,"","Changes the speed of color rotation.");
	freezebut.set(clearbut.x,clearbut.x+145,speedcbox.yend+10,speedcbox.yend+30,"Freeze/Unfreeze","Toggles Freeze/Unfreeze mode.  When this mode is enabled, clicking points toggles their frozen/free status.");
	originbut.set(clearbut.x,clearbut.x+120,freezebut.yend+10,freezebut.yend+30,"Show Origin","Toggles visibility of the plot origin as a white cross.");
	
	fsbut.set(winw-30,winw,0,20,"FS","Full Screen mode.  This actually only minimizes the toolbar.  You'll still have to maximize the program window if you want a truly full-screen display.");
	
	rendwbox.set(renderscreen[0]+90,renderscreen[1]+15,20,55,20,100,1,10000,0,800,"Width","Changes the width of the render (in pixels).");
	lockratiobut.set(rendwbox.xend+15,rendwbox.xend+102,rendwbox.y,rendwbox.yend,"Lock ratio","Locks the ratio between the width and height.  Inital aspect ratio is determined by the aspect ratio of the area selected.");
	rendhbox.set(rendwbox.x,rendwbox.y+30,20,55,20,100,1,10000,0,800,"Height","Changes the height of the render (in pixels).");
	rendibox.set(rendwbox.x,rendhbox.y+30,20,35,20,1,1,100,0,20,"Iterations","Changes the number of iterations of the render.  Be warned: there's no way to hault rendering, once it has begun, without terminating the program.");
	temptext.clear();
	temptext.push_back("BMP");
	temptext.push_back("PPM");
	ftypetog.set(rendwbox.xend+20,rendwbox.xend+20+50,rendhbox.y,rendhbox.yend,1,temptext,"Toggles format of rendered image.");
	rendnamefield.set(rendwbox.x,rendibox.y+30,rendwbox.x+170,rendibox.yend+30,2,"The name of the rendered image.  File extension should not be included.");
	startrenderbut.set(rendnamefield.x,rendnamefield.x+52,rendnamefield.y+30,rendnamefield.y+50,"Start","Starts the rendering process.");
	
	render(1);
	render(2);
}

void mouseaction(int button,int state,int x,int y)
{
	if(button==GLUT_LEFT_BUTTON&&state==GLUT_DOWN&&vars.dragpoint==-1)
	{
		if(renderbut.enabled&&helpmode==0)
		{
			if(renderbut.click(x,y))
			{
				renderprogress=0;
				renderarea[0][0]=renderarea[0][1]=renderarea[1][0]=renderarea[1][1]=-1;
				renderareaselect=0;
			}
			if(renderareaselect)
			{
				if(helpmode==0)
				{
					char tempchar[10];
					if(rendwbox.click(x,y)!=0)
					{
						if(lockratiobut.enabled)
						{
							rendhbox.val=rendwbox.val*renderratio;
							sprintf(tempchar,"%d",(int)rendhbox.val);
							rendhbox.field.text=tempchar;
						}
						else renderratio=rendhbox.val/rendwbox.val;
					}
					if(rendhbox.click(x,y)!=0)
					{
						if(lockratiobut.enabled)
						{
							rendwbox.val=rendhbox.val/renderratio;
							sprintf(tempchar,"%d",(int)rendwbox.val);
							rendwbox.field.text=tempchar;
						}
						else renderratio=rendhbox.val/rendwbox.val;
					}
					rendibox.click(x,y);
					if(lockratiobut.click(x,y))
						renderratio=rendhbox.val/rendwbox.val;
					rendnamefield.click(x,y);
					ftypetog.click(x,y);
					if(startrenderbut.click(x,y))renderimage();
				}
			}
			else if(checkrange(x,tbwidth+5,winw-5)&&checkrange(y,5,winh-5))
			{
				renderarea[0][0]=x;
				renderarea[0][1]=y;
			}
			render(4);
		}
		else if(helpmode==0)
		{
			toberend=1;
			
			if(clearbut.click(x,y))vars.point.clear();
			editbut.click(x,y);
			prevbut.click(x,y);
			if(itbox.click(x,y))
				if(vars.checkitnum()==1)toberend=0;
			if(playpbut.enabled)speedpbox.click(x,y);
			if(playcbut.enabled)speedcbox.click(x,y);
			ptsizebox.click(x,y);
			cdfbox.click(x,y);
			blendbox.click(x,y);
			cobox.click(x,y);
			playpbut.click(x,y);
			playcbut.click(x,y);
			if(rpnumbox.click(x,y))vars.checkitnum();
			symbut.click(x,y);
			flipbut.click(x,y);
			if(evenbut.click(x,y))vars.checkitnum();
			if(oddbut.click(x,y))vars.checkitnum();
			depibut.click(x,y);
			deppbut.click(x,y);
			freezebut.click(x,y);
			mod1but.click(x,y);
			mod2but.click(x,y);
			fsbut.click(x,y);
			originbut.click(x,y);
	
			if(editbut.enabled&&x>tbwidth&&!fsbut.inrange(x,y))
			{
				if(vars.overpoint(x,y)==-1)
				{
					if(glutGetModifiers()==GLUT_ACTIVE_CTRL)
						vars.newpoint(x,y,false);
					else vars.newpoint(x,y,true);
					vars.checkitnum();
					vars.dragpoint=vars.point.size()-1;
				}
				if(freezebut.enabled||glutGetModifiers()==GLUT_ACTIVE_SHIFT)vars.point[vars.overpoint(x,y)].free=!vars.point[vars.overpoint(x,y)].free;
				else vars.dragpoint=vars.overpoint(x,y);
			}
	
			if(renderbut.click(x,y))
			{
				char tempchar[10];
				renderareaselect=false;
				rendibox.val=itbox.val+1;
				sprintf(tempchar,"%d",(int)rendibox.val);
				rendibox.field.text=tempchar;
				
				srand(time(NULL));
				char str[100];
				sprintf(str,"Render%04d",rand()%10000);
				rendnamefield.text=str;
				
				render(4);
			}
			else
			{
				render(2);
			}
		}
		else infobut.click(x,y);
		if(helpbut.click(x,y))
		{
			infobut.enabled=0;
			if(helpmode)helpmode=0;
			else helpmode=1;
		}
	}
	else if(button==GLUT_LEFT_BUTTON&&state==GLUT_UP)
	{
		if(renderbut.enabled)
		{
			if(renderareaselect==false&&checkrange(x,tbwidth+5,winw-5)&&checkrange(y,5,winh-5))
			{
				char tempchar[10];
				float tempf;
				if(renderarea[0][0]>-1)
				{
					renderarea[1][0]=x;
					renderarea[1][1]=y;
					renderareaselect=true;
				}
				if(renderarea[0][0]>renderarea[1][0])
				{
					tempf=renderarea[1][0];
					renderarea[1][0]=renderarea[0][0];
					renderarea[0][0]=tempf;
				}
				if(renderarea[0][1]>renderarea[1][1])
				{
					tempf=renderarea[1][1];
					renderarea[1][1]=renderarea[0][1];
					renderarea[0][1]=tempf;
				}
				renderratio=(renderarea[1][1]-renderarea[0][1])/(renderarea[1][0]-renderarea[0][0]);
				rendwbox.val=800;
				rendhbox.val=800*renderratio;
				sprintf(tempchar,"%d",(int)rendwbox.val);
				rendwbox.field.text=tempchar;
				sprintf(tempchar,"%d",(int)rendhbox.val);
				rendhbox.field.text=tempchar;
				render(4);
			}
		}
		else
		{
			if(clearbut.enabled)clearbut.enabled=0;
			vars.dragpoint=-1;
		}
	}
	else if(button==GLUT_RIGHT_BUTTON&&state==GLUT_DOWN&&renderbut.enabled==0)
		if(editbut.enabled&&vars.overpoint(x,y)>-1&&helpmode==0)
		{
			if(vars.deletepoint(vars.overpoint(x,y)));
			toberend=1;
			render(2);
		}
}

void mousemotion(int x,int y)
{
	mouseloc[0]=x;
	mouseloc[1]=y;
	if(vars.dragpoint>-1)
	{
		plotvala(x,y,vars.point[vars.dragpoint].x);
		render(1);
		render(2);
	}
	if(renderbut.enabled==0)render(2);
}

void controls(unsigned char key,int x,int y)
{
	if(key=='=')
	{
		plotradius[0]*=.8;
		plotradius[1]*=.8;
		render(1);
		render(2);
	}
	else if(key=='-')
	{
		plotradius[0]/=.8;
		plotradius[1]/=.8;
		render(1);
		render(2);
	}
	else if(key==27)//escape
	{
		if(renderbut.enabled&&renderareaselect==false)
		{
			renderarea[0][0]=renderarea[0][1]=-1;
			renderbut.enabled=0;
			render(4);
		}
	}
	else
	{
		if(renderbut.enabled&&startrenderbut.enabled==false)
		{
			char tempchar[10];
			rendnamefield.type(key);
			if(rendwbox.type(key))
			{
				if(lockratiobut.enabled)
				{
					rendhbox.val=rendwbox.val*renderratio;
					sprintf(tempchar,"%d",(int)rendhbox.val);
					rendhbox.field.text=tempchar;
				}
				else renderratio=rendhbox.val/rendwbox.val;
			}
			if(rendhbox.type(key))
			{
				if(lockratiobut.enabled)
				{
					rendwbox.val=rendhbox.val/renderratio;
					sprintf(tempchar,"%d",(int)rendwbox.val);
					rendwbox.field.text=tempchar;
				}
				else renderratio=rendhbox.val/rendwbox.val;
			}
			rendibox.type(key);
			render(4);
		}
		else
		{
			if(itbox.type(key))render(1);
			if(ptsizebox.type(key))render(1);
			if(cdfbox.type(key))render(1);
			if(cobox.type(key))render(1);
			if(blendbox.type(key))render(1);
			speedpbox.type(key);
			speedcbox.type(key);
		}
	}
}

void specialcontrols(int key,int x,int y)
{
	if(renderbut.enabled==true)
	{
		if(rendwbox.field.enabled)rendwbox.field.shiftcursor(key);
		else if(rendhbox.field.enabled)rendhbox.field.shiftcursor(key);
		else if(rendibox.field.enabled)rendibox.field.shiftcursor(key);
		else if(rendnamefield.enabled)rendnamefield.shiftcursor(key);
	}
	else if(itbox.field.enabled)itbox.field.shiftcursor(key);
	else if(ptsizebox.field.enabled)ptsizebox.field.shiftcursor(key);
	else if(cdfbox.field.enabled)cdfbox.field.shiftcursor(key);
	else if(cobox.field.enabled)cobox.field.shiftcursor(key);
	else if(blendbox.field.enabled)blendbox.field.shiftcursor(key);
	else if(speedpbox.field.enabled)speedpbox.field.shiftcursor(key);
	else if(speedcbox.field.enabled)speedcbox.field.shiftcursor(key);
	else if(renderbut.enabled==false)
	{
		if(key==GLUT_KEY_LEFT)
			origin[0]-=plotradius[0]/40.0;
		else if(key==GLUT_KEY_RIGHT)
			origin[0]+=plotradius[0]/40.0;
		if(key==GLUT_KEY_UP)
			origin[1]-=plotradius[1]/40.0;
		else if(key==GLUT_KEY_DOWN)
			origin[1]+=plotradius[1]/40.0;
		render(1);
		render(2);
	}
}			

void display()
{
	int i;
	glClear(GL_COLOR_BUFFER_BIT);
	
	if(renderbut.enabled==0)
	{
		if(playpbut.enabled)
		{
			vars.pointmotion();
			toberend=2;
			render(2);
		}
		if(playcbut.enabled)
		{
			vars.comotion+=speedcbox.val/200.0;
			toberend=2;
			render(2);
		}
	}
	else render(4);
	
	render(3);
	if(toberend==2)render(1);
	else if(toberend==1)
	{
		glColor3f(1,1,1);
		dispstring(tbwidth+50,40,"Calculating...");
		toberend=2;
	}
	
	glCallList(1);
	glCallList(2);
	glCallList(3);
	
	if(renderbut.enabled)
	{
		if(rendnamefield.enabled)render(4);
		glCallList(4);
	}
	glFlush();
}

int main(int argc,char **argv)
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
	glutInitWindowSize(winw,winh);
	glutInitWindowPosition(100,10);
	glutCreateWindow("Recursion Lab 0.2.01 beta");
	init();

	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutReshapeFunc(resize);
	glutMouseFunc(mouseaction);
	glutMotionFunc(mousemotion);
	glutPassiveMotionFunc(mousemotion);
	glutKeyboardFunc(controls);
	glutSpecialFunc(specialcontrols);
	glutMainLoop();
	return 1;
}
					
