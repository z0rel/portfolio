import { BuildInPlacements } from 'rc-trigger/lib/interface';

export const DROPDOWN_TOP_ALIGN: BuildInPlacements = ({
  // move point of source node to align with point of target node.
  points: ['bl', 'tl'], // bottom left of source node with top left of target node
  offset: [0, -4], // offset target node by [x, y]
  overflow: {
    adjustX: 0,
    adjustY: 1, // will adjust source node in y direction if source node is invisible.
  },
} as BuildInPlacements);

export const DROPDOWN_BOTTOM_ALIGN: BuildInPlacements = ({
  // move point of source node to align with point of target node.
  points: ['tl', 'bl'], // top left of source node with bottom left of target node
  offset: [0, 4], // offset target node by [x, y]
  overflow: {
    adjustX: 1, // will adjust source node in x direction if source node is invisible.
    adjustY: 0,
  },
} as BuildInPlacements);



