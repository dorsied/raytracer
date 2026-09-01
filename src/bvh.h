#ifndef BVH_H
#define BVH_H

#include "hittable.h"
#include "hittable_list.h"
#include "aabb.h"
#include <algorithm>
#include <vector>

class bvh_node : public hittable {
  public:
    // build BVH from a hittable_list
    bvh_node(hittable_list& list) : bvh_node(list.objects, 0, list.objects.size()) {}

    // recursive constructor: splits objects[start..end) using SAH
    bvh_node(std::vector<std::shared_ptr<hittable>>& objects, size_t start, size_t end){
        size_t count = end - start;

        // compute enclosing AABB for this node
        box = aabb();
        for (size_t i = start; i < end; i++)
            box = aabb::merge(box, objects[i]->bounding_box());

        // Leaf cases
        if (count == 1) {
            left = right = objects[start];
            return;
        }
        if (count == 2) {
            left  = objects[start];
            right = objects[start + 1];
            return;
        }

        // --- SAH split ---
        // For each axis sweep through sorted primitives + evaluate split cost
        // Cost = 1 + (SA_left * N_left + SA_right * N_right) / SA_parent
        // (traversal cost = 1, intersection cost = 1, factored out)

        double parent_sa = box.surface_area();
        double best_cost = infinity;
        int best_axis = 0;
        size_t best_split = start + count / 2;

        for (int axis = 0; axis < 3; axis++) {
            // Sort by centroid along this axis
            std::sort(objects.begin() + start, objects.begin() + end, [axis](const std::shared_ptr<hittable>& a, const std::shared_ptr<hittable>& b) {
                    return a->bounding_box().centroid()[axis] < b->bounding_box().centroid()[axis];
                });

            // Prefix - cover objects[start .. start+i]
            std::vector<aabb> prefix(count);
            prefix[0] = objects[start]->bounding_box();
            for (size_t i = 1; i < count; i++) {
                prefix[i] = aabb::merge(prefix[i - 1], objects[start + i]->bounding_box());
            }

            // Suffix - cover objects[start+i .. end)
            std::vector<aabb> suffix(count);
            suffix[count - 1] = objects[end - 1]->bounding_box();
            for (int i = (int)count - 2; i >= 0; i--){
                suffix[i] = aabb::merge(suffix[i + 1], objects[start + i]->bounding_box());
            }

            // Evaluate every possible split position
            // split k means: left=[0..k-1], right=[k..count-1], k in [1, count-1]
            for (size_t k = 1; k < count; k++) {
                double sa_left  = prefix[k - 1].surface_area();
                double sa_right = suffix[k].surface_area();
                double cost = 1.0
                    + (sa_left  * (double)k
                    +  sa_right * (double)(count - k)) / parent_sa;

                if (cost < best_cost) {
                    best_cost  = cost;
                    best_axis  = axis;
                    best_split = start + k;
                }
            }
        }

        // re-sort by the chosen axis
        if (best_axis != 2) {
            std::sort(objects.begin() + start, objects.begin() + end,
                [best_axis](const std::shared_ptr<hittable>& a,
                            const std::shared_ptr<hittable>& b) {
                    return a->bounding_box().centroid()[best_axis]
                         < b->bounding_box().centroid()[best_axis];
                });
        }

        left = std::make_shared<bvh_node>(objects, start, best_split);
        right = std::make_shared<bvh_node>(objects, best_split, end);
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        if (!box.hit(r, ray_t)) return false;

        bool hit_left  = left->hit(r, ray_t, rec);
        bool hit_right = right->hit(
            r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);

        return hit_left || hit_right;
    }

    aabb bounding_box() const override { return box; }

  private:
    std::shared_ptr<hittable> left;
    std::shared_ptr<hittable> right;
    aabb box;
};

#endif