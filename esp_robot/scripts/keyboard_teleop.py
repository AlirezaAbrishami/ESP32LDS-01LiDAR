#!/usr/bin/env python

import rospy
from geometry_msgs.msg import Twist
from pynput import keyboard

pub = rospy.Publisher('cmd_vel', Twist, queue_size=1)
move_cmd = Twist()

speed = 1.0
forward = False
backward = False
right_forward = False
left_forward = False
right_backward = False
left_backward = False

def talker():
    rospy.init_node('talker', anonymous=True)

    move_cmd.linear.x = 1.0

def on_press(key):
    print(str(key))
    global pub
    global move_cmd
    global speed
    
    global forward
    global backward
    global right_forward
    global left_forward
    global right_backward
    global left_backward

    if str(key) == "u\'w\'" and not forward:
        move_cmd.linear.y = speed
        move_cmd.linear.x = 0.0
        forward = True
        rospy.sleep(0.1)
        pub.publish(move_cmd)
    elif str(key) == "u\'s\'" and not backward:
        move_cmd.linear.y = -speed
        move_cmd.linear.x = 0.0
        backward = True
        rospy.sleep(0.1)
        pub.publish(move_cmd)
    elif str(key) == "u\'d\'" and not right_forward:
        move_cmd.linear.y = speed / 2
        move_cmd.linear.x = -speed
        right_forward = True
        rospy.sleep(0.1)
        pub.publish(move_cmd)
    elif str(key) == "u\'a\'" and not left_forward:
        move_cmd.linear.y = speed / 2
        move_cmd.linear.x = speed
        left_forward = True
        rospy.sleep(0.1)
        pub.publish(move_cmd)
    elif str(key) == "u\'D\'" and not right_backward:
        move_cmd.linear.y = -speed / 2
        move_cmd.linear.x = -speed
        right_backward = True
        rospy.sleep(0.1)
        pub.publish(move_cmd)
    elif str(key) == "u\'A\'" and not left_backward:
        move_cmd.linear.y = -speed / 2
        move_cmd.linear.x = speed
        left_backward = True
        rospy.sleep(0.1)
        pub.publish(move_cmd)

def on_release(key):
    global forward
    global backward
    global right_forward
    global left_forward
    global right_backward
    global left_backward
    
    global move_cmd
    global pub

    forward = False; backward = False; right_forward = False; left_forward = False; right_backward = False; left_backward = False

    move_cmd.linear.x = 0.0
    move_cmd.linear.y = 0.0
    rospy.sleep(0.1)
    pub.publish(move_cmd)


if __name__ == '__main__':
    try:
        talker()
        with keyboard.Listener(
        on_press=on_press,
        on_release=on_release) as listener:
            listener.join()
    except rospy.ROSInterruptException:
        pass

