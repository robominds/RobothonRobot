
struct way_points_str {
  int entry;
  int cord[2];
};

struct way_plan_str {
  int entry;
  int type;
  int way_point;
  int next;
  int next_ret;
  int tol;
  int wall;
};

