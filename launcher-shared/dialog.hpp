#pragma once

#include "engine/engine.hpp"

struct Dialog {
  std::string title, message;

  using AnswerFunc = void(*)(bool);
  AnswerFunc on_answer;

  void show(std::string title, std::string message, AnswerFunc on_answer) {
    this->title = title;
    this->message = message;
    this->on_answer = on_answer;
  }

  bool update() {
    if(title.empty())
      return false;

    if(buttons.released & Button::A) {
      on_answer(true);
      title = message = "";
    } else if(buttons.released & Button::B) {
      on_answer(false);
      title = message = "";
    }

    return true;
  }

  void button_icon(const Point &pos, int button) {
    Pen active_col(0xff, 0xff, 0xff);
    Pen col(0x50, 0x64, 0x78);
    Rect r(pos.x, pos.y + 3, 2, 2);

    screen.pen = button == Button::Y ? active_col : col;
    screen.rectangle(r);

    screen.pen = button == Button::B ? active_col : col;
    r.x += 6;
    screen.rectangle(r);

    screen.pen = button == Button::X ? active_col : col;
    r.x -= 3; r.y = pos.y;
    screen.rectangle(r);

    if(button == Button::A) screen.pen = active_col;
    r.y += 6;
    screen.rectangle(r);
  }

  void draw() {
    if(title.empty())
      return;

    const Rect dialog_rect(45, 77, 230, 85);
    const int header_height = 16;

    screen.pen = Pen(235, 245, 255);
    screen.rectangle(dialog_rect);

    screen.pen = Pen(3, 5, 7);
    screen.rectangle(Rect(dialog_rect.x + 1, dialog_rect.y + header_height, dialog_rect.w - 2, dialog_rect.h - header_height - 1));

    screen.text(title, minimal_font, Rect(dialog_rect.x + 3, dialog_rect.y, dialog_rect.w - 6, header_height + minimal_font.spacing_y), true, TextAlign::center_left);

    screen.pen = Pen(255, 255, 255);
    screen.text(message, minimal_font, Rect(dialog_rect.x + 6, dialog_rect.y + header_height + 5, dialog_rect.w - 12, 45));

    screen.text("No      Yes    ", minimal_font, Rect(dialog_rect.x + 1, dialog_rect.y + dialog_rect.h - 17, dialog_rect.w - 2, 16 + minimal_font.spacing_y), true, TextAlign::center_right);

    button_icon(Point(dialog_rect.x + 185, dialog_rect.y + 71), Button::B);
    button_icon(Point(dialog_rect.x + 218, dialog_rect.y + 71), Button::A);
  }

};