#ifndef TILEBUTTON_HPP_INCLUDED
#define TILEBUTTON_HPP_INCLUDED

#include "AppResources.hpp"
#include "BaseDrawWindow.hpp"
#include <string>

class TileButton {

public:
	typedef bool (BaseDrawWindow::*OnClickMethod)(Message&);

    TileButton(ingenico::graphics::Widget& parent,
               const std::string& bgPng,
               const std::string& iconPng,
               const std::string& labelTxt,
               int posX, int posY, int id,
			   BaseDrawWindow* target = 0,
			   OnClickMethod method = 0);

    ~TileButton();

    bool handleClick(Message& msg);

    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    Button* getButton() const{return background;}
    Picture* getBack() const{return back;}


private:
    ingenico::graphics::Button* background;
    ingenico::graphics::Picture* icon;
    ingenico::graphics::Picture* back;
    ingenico::graphics::Label* label;
    int x, y, width, height;
    BaseDrawWindow* target_;
    OnClickMethod   method_;

};

#endif // TILEBUTTON_HPP_INCLUDED
