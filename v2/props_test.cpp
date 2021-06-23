#include "props2.h"

#include <string>
using std::string;

int main(int argc, char **argv) {
    PropertyNode root_node = PropertyNode("/", true);
    PropertyNode t1_node = root_node.getChild("task", true);
    PropertyNode task_node = t1_node.getChild("home", true);
    
    for ( int i = 0; i < 1000; i++ ) {
	printf("%d\n", i);
	PropertyNode root_node = PropertyNode("/", true);
	PropertyNode t1_node = root_node.getChild("task", true);
	PropertyNode task_node = t1_node.getChild("home", true);
    }

    printf("making imu_node: /sensors/imu/2\n");
    PropertyNode imu_node = PropertyNode("/sensors/imu/2", true);
    printf("before calling getDouble()\n");
    printf("az = %.2f\n", imu_node.getDouble("az"));
    printf("az(int) = %ld\n", imu_node.getInt("az"));
    imu_node.setDouble("az", -9.7);
    printf("az = %.2f\n", imu_node.getDouble("az"));
    imu_node.setInt("az", -10);
    printf("az = %.2f\n", imu_node.getDouble("az"));
    imu_node.setString("az", "-9.8092322");
    printf("az = %.8f\n", imu_node.getDouble("az"));
   
    PropertyNode gps_node_5 = PropertyNode("/sensors/gps/5", true);
    printf("gps name = %s\n", gps_node_5.getString("name").c_str());
    printf("gps test = %f\n", gps_node_5.getDouble("test1"));
    printf("gps test = %ld\n", gps_node_5.getInt("test1"));
    printf("gps test = %s\n", gps_node_5.getString("test1").c_str());

    PropertyNode sensors_node = PropertyNode("/sensors");
    printf("gps size = %d\n", sensors_node.getLen("gps"));

    PropertyNode("/sensors/imu/2");
    PropertyNode("/sensors/gps/8", true);

    PropertyNode p("/sensors", true);
    string val = p.getString("imu/0");
    printf("/sensors/imu = %s\n", val.c_str());
    // this is nonsensical usage, but also causes a segfault which we
    // really need to catch
    PropertyNode t1 = PropertyNode("/sensors/imu/1", true);
    string t2 = t1.getString("az");
    printf("t1.t2=%s\n", t2.c_str());
    printf("/sensors/imu/1 = %s\n", PropertyNode("/sensors/imu/1").getString("az").c_str());

    string fullpath = "/sensors/imu/1/az";
    int pos = fullpath.rfind("/");
    printf("%d %s / %s\n", pos, fullpath.substr(0, pos).c_str(),
	   fullpath.substr(pos+1).c_str() );

    PropertyNode sensors1 = PropertyNode("/sensors", true);
    vector <string> children = sensors1.getChildren();
    unsigned int count = children.size();
    for ( unsigned int i = 0; i < count; i++ ) {
	PropertyNode node = sensors1.getChild(children[i].c_str());
	printf("%d %s\n", i, children[i].c_str());
    }

    PropertyNode device = PropertyNode("/sensors/device", true);
    device.setDouble("var1", 12.345);

    PropertyNode device0 = PropertyNode("/sensors/device/0", true);
    device0.setDouble("var2", 54.321);

    device.setDouble("var3", 99);
    device0.setDouble("var4", 88);
    
    children = sensors_node.getChildren("gps");
    for ( unsigned int i = 0; i < children.size(); i++ ) {
	printf("sensor child = %s\n", children[i].c_str());
    }

    PropertyNode("/").pretty_print();
}
