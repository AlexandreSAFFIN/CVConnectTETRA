#ifndef TILEBUTTON_HPP_INCLUDED
#define TILEBUTTON_HPP_INCLUDED

#include "AppResources.hpp"
#include <string>

class TileButton {
private:
    ingenico::graphics::Button* background;
    ingenico::graphics::Picture* icon;
    ingenico::graphics::Picture* back;
    ingenico::graphics::Label* label;
    int x, y, width, height;

public:
    TileButton(ingenico::graphics::Widget& parent,
               const std::string& bgPng,
               const std::string& iconPng,
               const std::string& labelTxt,
               int posX, int posY);

    ~TileButton();


    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    Button* getButton() const{return background;}
    Picture* getBack() const{return back;}
};

#endif // TILEBUTTON_HPP_INCLUDED
