import React, { useState } from 'react';
import { Resizable } from 'react-resizable';

export const ResizableTitle = ({ onResize, width, onClick, ...restProps }) => {
  const [allowClick, setAllowClick] = useState(true);

  if (!width) {
    return <th {...restProps} />;
  }

  return (
    <Resizable
      width={width}
      height={0}
      handle={
        <span
          className="react-resizable-handle"
          onClick={(e) => {
            e.stopPropagation();
          }}
        />
      }
      onResize={onResize}
      onMouseDown={(e) => {
        setAllowClick(true);
      }}
      onResizeStart={(e) => {
        setAllowClick(false);
      }}
      onClick={(e) => allowClick && onClick && onClick(e)}
      draggableOpts={{ enableUserSelectHack: false }}
    >
      <th {...restProps} />
    </Resizable>
  );
};

