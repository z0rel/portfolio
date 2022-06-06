export class TableaPageInfo {
  constructor(page, pageSize) {
    this.page = page;
    if (typeof pageSize === 'string') {
      pageSize = parseInt(pageSize);
    }
    this.pageSize = pageSize;
  }

  getSlicedData(data, pagination, choosedBlock, frontendPagination) {
    if (!this.pageSize || this.pageSize <= 0) {
      return data;
    }
    if (!frontendPagination) {
      return Array.prototype.slice.call(data, 0, this.pageSize);
    }
    else {
      let pageNum = (pagination[choosedBlock]?.page || 1) - 1;
      let offset = pageNum * this.pageSize;
      return Array.prototype.slice.call(data, offset, offset + this.pageSize);
    }
  }

  // Получить количество показываемых элементов "по"
  getToShowItems(pagination, choosedBlock, frontendPagination) {
    if (!frontendPagination) {
      return this.pageSize && this.pageSize > 0 ? this.pageSize : 0;
    }
    else {
      return this.pageSize && this.pageSize > 0 ? this.pageSize : 0;
    }
  }
}
