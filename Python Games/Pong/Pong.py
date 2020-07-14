# -*- coding: utf-8 -*-
"""
Created on Sun Apr 19 12:37:36 2020

@author: Zazi 2
"""

import turtle
from playsound import playsound
import time


def quit_game():
    wn = turtle.Screen()
    wn.bye()


def start_game():
    wn = turtle.Screen()
    wn.title("Pong by Norbert Csornai")
    wn.bgcolor("black")
    wn.setup(width=800, height=600)
    wn.tracer(0)

    # Score
    score_a = 0
    score_b = 0

    # Paddle A
    paddle_a = turtle.Turtle()
    paddle_a.speed(0)
    paddle_a.shape("square")
    paddle_a.color("white")
    paddle_a.shapesize(5, 1)
    paddle_a.penup()
    paddle_a.goto(-350, 0)

    # Paddle B
    paddle_b = turtle.Turtle()
    paddle_b.speed(0)
    paddle_b.shape("square")
    paddle_b.color("white")
    paddle_b.shapesize(5, 1)
    paddle_b.penup()
    paddle_b.goto(350, 0)

    # Ball
    ball = turtle.Turtle()
    ball.speed(0)
    ball.shape("square")
    ball.color("white")
    ball.penup()
    ball.goto(0, 0)
    ball.dx = 0.15
    ball.dy = -0.15

    # Pen
    pen = turtle.Turtle()
    pen.speed(0)
    pen.color("white")
    pen.penup()
    pen.hideturtle()
    pen.goto(0, 260)
    pen.write("Player A: 0   Player B: 0", align="center", font=("Courier", 24, "normal"))

    # Functions
    def paddle_a_up():
        y = paddle_a.ycor()
        y += 20
        paddle_a.sety(y)

    def paddle_a_down():
        y = paddle_a.ycor()
        y -= 20
        paddle_a.sety(y)

    def paddle_b_up():
        y = paddle_b.ycor()
        y += 20
        paddle_b.sety(y)

    def paddle_b_down():
        y = paddle_b.ycor()
        y -= 20
        paddle_b.sety(y)

    # keyboard bindings
    wn.listen()
    wn.onkeypress(paddle_a_up, "w")
    wn.onkeypress(paddle_a_down, "s")
    wn.onkeypress(paddle_b_up, "Up")
    wn.onkeypress(paddle_b_down, "Down")

    # Main game logo

    # while True:
    while score_a < 5 and score_b < 5:

        wn.update()

        # Move the ball
        ball.setx(ball.xcor() + ball.dx)
        ball.sety(ball.ycor() + ball.dy)

        # Border checking
        if ball.ycor() > 290:
            ball.sety(290)
            ball.dy *= -1
            playsound('bounce.wav', False)

        if ball.ycor() < -290:
            ball.sety(-290)
            ball.dy *= -1
            playsound('bounce.wav', False)

        if ball.xcor() > 390:
            ball.goto(0, 0)
            ball.dx *= -1
            score_a += 1
            pen.clear()
            pen.write("Player A: {}   Player B: {}".format(score_a, score_b), align="center",
                      font=("Courier", 24, "normal"))

        if ball.xcor() < -390:
            ball.goto(0, 0)
            ball.dx *= -1
            score_b += 1
            pen.clear()
            pen.write("Player A: {}   Player B: {}".format(score_a, score_b), align="center",
                      font=("Courier", 24, "normal"))

        # Paddle and ball coalitions
        if (340 < ball.xcor() < 350) and (
                paddle_b.ycor() + 40 > ball.ycor() > paddle_b.ycor() - 40):
            playsound('bounce.wav', False)
            ball.setx(340)
            ball.dx *= -1

        if (-340 > ball.xcor() > -350) and (
                paddle_a.ycor() + 40 > ball.ycor() > paddle_a.ycor() - 40):
            playsound('bounce.wav', False)
            ball.setx(-340)
            ball.dx *= -1

        if score_a == 5:
            pen.clear()
            pen.write("Player A won the game!", align="center",
                      font=("Courier", 24, "normal"))
        elif score_b == 5:
            pen.clear()
            pen.write("Player B won the game!", align="center",
                      font=("Courier", 24, "normal"))


def main():
    # mainwn = turtle.Screen()
    # mainwn.title("Pong by Norbert Csornai")
    # mainwn.bgcolor("black")
    # mainwn.setup(width=800, height=600)
    # mainwn.tracer(0)
    #
    # pen = turtle.Turtle()
    # pen.speed(0)
    # pen.color("white")
    # pen.penup()
    # pen.hideturtle()
    # pen.goto(0, -250)
    # pen.write("Press 1 to Start 2 to Quit", align="center", font=("Courier", 15, "normal"))
    #
    # mainwn.listen()
    # mainwn.onkeypress(start_game, "1")
    # mainwn.onkeypress(quit_game, "2")
    #
    # mainwn.mainloop()

    start_game()


if __name__ == "__main__":
    main()
