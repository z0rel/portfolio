
const RingitemWork = ({content}) => {
    return <li className='ringitem' itemProp="offers" itemScope itemType="http://schema.org/Offer">
        <div className="item" itemProp="name"><p dangerouslySetInnerHTML={{__html: content}}></p></div>
    </li>
}

export default RingitemWork