SCString GenTag(SCStudyInterfaceRef sc)
{
	SCString Buffer;
	SCString Chartbook   = sc.ChartbookName();
	int      IdStudy     = sc.StudyGraphInstanceID;
	int      ChartNumber = sc.ChartNumber;
	int      IdProcess   = sc.ProcessIdentifier;
	
	Buffer.Format("%d-%d-%s-%d", IdStudy, ChartNumber, Chartbook.GetChars(), IdProcess);

	return Buffer;
}

int NewOrder(SCStudyInterfaceRef sc, int& id, BuySellEnum dir, int qty, int type, float price, SCString tag, SCTimeInForceEnum tif = SCT_TIF_GOOD_TILL_CANCELED)
{
	s_SCNewOrder NewOrder;
	
	NewOrder.OrderQuantity = qty;
	NewOrder.OrderType     = type;
	NewOrder.TimeInForce   = tif;
	NewOrder.Price1        = price;
	NewOrder.TextTag       = tag;
	
	int AcceptedOrder = dir == BSE_BUY ? sc.BuyOrder(NewOrder) : dir == BSE_SELL ? sc.SellOrder(NewOrder) : -1;
	
	if (AcceptedOrder > 0)
	{
		id = NewOrder.InternalOrderID;
	}
	
	return AcceptedOrder;
}

int StrategyOrder(SCStudyInterfaceRef sc, int& id, BuySellEnum dir, int qty, int type, float price, SCString tag, SCTimeInForceEnum tif = SCT_TIF_GOOD_TILL_CANCELED, bool replace = false)
{
	int            Result = -1; 
	s_SCTradeOrder Order;
	
	sc.GetOrderByOrderID(id, Order);

	if (IsWorkingOrderStatus(Order.OrderStatusCode))
	{	
		if (Order.Price1 != price or Order.OrderQuantity != qty or Order.OrderTypeAsInt != type)
		{
			if (Order.OrderTypeAsInt == type and Order.BuySell == dir and not replace)
			{
				s_SCNewOrder ModifyOrder;
				
				ModifyOrder.OrderQuantity   = qty;
				ModifyOrder.InternalOrderID = id;
				ModifyOrder.Price1          = price;

				int AcceptedMod = sc.ModifyOrder(ModifyOrder);
				
				if (AcceptedMod > 0)
				{
					id = ModifyOrder.InternalOrderID;
				}
				
				Result = AcceptedMod;
			}
			else
			{
				int AcceptedCancel = sc.CancelOrder(id);
			
				if (AcceptedCancel > 0)
				{									
					Result = NewOrder(sc, id, dir, qty, type, price, tag);
				}
			}
			
		}
	}
	else
	{
		Result = NewOrder(sc, id, dir, qty, type, price, tag);
	}
	
	return Result;
}

int StrategyCancel(SCStudyInterfaceRef sc, int id)
{
	int            Result = -1;
	s_SCTradeOrder Order;

	sc.GetOrderByOrderID(id, Order);
	
	if (IsWorkingOrderStatus(Order.OrderStatusCode))
	{
		int Result = sc.CancelOrder(id);	
	}
	
	return Result;
}
