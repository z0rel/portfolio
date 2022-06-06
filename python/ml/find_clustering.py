
import statistics
import numpy as np
from itertools import chain
from sklearn.cluster import DBSCAN


class Clustering:
    def __init__(self, k, stat):
        self.format_clusters_len_max = 0
        self.k = k
        self.stat = stat
        median = statistics.median(stat)
        pstdev = statistics.pstdev(stat)
        self.median = int(median)
        self.pstdev = int(pstdev)
        self.relative_pstdev = 100.0 * pstdev / median

        m = float(max(stat))
        X = np.array([float(i) / m for i in stat])
        stat = [float(i) for i in stat]
        X = X.reshape(-1, 1)
        return
        db = DBSCAN(eps=0.1, min_samples=1).fit(X)

        core_mask = np.zeros_like(db.labels_, dtype=bool)
        core_mask[db.core_sample_indices_] = True

        self.clusters = []
        for cluster in set(db.labels_):
            mask = (db.labels_ == cluster)
            cluster_data = [int(stat[i]) for i, f in enumerate(mask) if f]
            centroid = round(statistics.median(cluster_data))
            pv = statistics.pstdev(cluster_data)

            self.clusters.append((len(cluster_data),
                                  round(100 * len(cluster_data) / len(stat)),
                                  centroid,
                                  round((pv / centroid) * 100.0)))
        self.clusters = list(reversed(sorted(self.clusters)))

    def __str__(self):
        return '"{0}": {{"m":{1:7.3f}, "pv%":{2:3}}}'.format(self.k, self.median / 1000.0, round(self.relative_pstdev))

    def format_clusters_list(self):
        return [] # [('{0:3}%: {1}' + ('/{2}%' if pv else '/0%')).format(l_per, val, pv) for (l, l_per, val, pv) in self.clusters]

    def format_clusters_balance_fmt1(self):
        return '{0:10} '

    def format_clusters_balance_fmt2(self):
        return "{0:" + str(self.format_clusters_len_max) + "}"

    def format_clusters(self):
        l_s = [self.format_clusters_balance_fmt2().format(s) for s in self.format_clusters_list()]
        return self.format_clusters_balance_fmt1().format(self.k) + ','.join(l_s)

    @staticmethod
    def balance_fmt_clusters(measurments):
        return
        format_clusters_len_max = max([len(s) for s in chain.from_iterable([m.format_clusters_list() for m in measurments])])
        for m in measurments:
            m.format_clusters_len_max = format_clusters_len_max

    @staticmethod
    def get_diff_fmt_clusters(l1, l2):
        return [c1[2] / c2[2] for c1, c2 in zip(l1.clusters, l2.clusters)]

    def format_clusters_diffs(self, diffs):
        l_s = [self.format_clusters_balance_fmt2().format('      {0:3.2f}'.format(d)) for d in diffs]
        return self.format_clusters_balance_fmt1().format('diff') + ','.join(l_s)
