#!/usr/bin/env python

import rospy
from geometry_msgs.msg import Twist

print('hello')
def talker():
    rospy.init_node('talker', anonymous=True)
    pub = rospy.Publisher('cmd_vel', Twist, queue_size=1)

    move_cmd = Twist()
    move_cmd.linear.x = 1.0
    rate = rospy.Rate(10)
    rate.sleep()
    pub.publish(move_cmd)

if __name__ == '__main__':
    try:
        talker()
    except rospy.ROSInterruptException:
        pass

