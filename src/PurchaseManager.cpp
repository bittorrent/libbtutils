#include "StdAfx.h"
#include "PurchaseManager.h"

//ut_core
#include "winmsgs.h"
#include "registry.h"
#include "Base64.h"
#include "BenchThread.h"

//ut_win
#include "verification_lib/plus_fingerprint.h"

//orderLib
#include <orderLib/Reconciliation.h>

//Registry keys.
#define TPRODUCT_PURCHASES_KEYPATH _T("Software\\") TCLIENTNAME _T("Purchases")
#define TPRODUCT_PURCHASEKEYS_KEYPATH _T("Software\\") TCLIENTNAME _T("Purchases\\PurchasedKeys")
#define TPRODUCT_CUSTOMERID_SUBKEY _T("CustomerId")

extern "C" void __cpuid(int a[4], int b);

extern HWND g_wndoverview;

//######################################################################
//For recording a newly purchased key.  Throws and does nothing if the key is not valid.
void PurchaseManager::saveAndActivateKey(const std::string &licenseKey) {
    //Make the order key.
    orderLib::Key keyObj(licenseKey);

    //Establish that it's good.  Will throw if it's bad.
    keyObj.validate();

    //Get the code.
    std::string code = keyObj.get("svcCode");
    tstring ts_code = to_tstring(code);

    //Get any old key that was there.
    tstring ts_oldKey;
    RegQueryString(ROOTKEY, TPRODUCT_PURCHASEKEYS_KEYPATH, ts_code.c_str(), ts_oldKey);

    //Load up the old key.  We don't care if it's valid, we only pass it in case there's old information to manage.
    string oldKey(to_string(ts_oldKey));
    orderLib::Key oldKeyObj(oldKey.c_str());

    //Set the service
    PurchaseManager::setServiceState(code, keyObj, oldKeyObj);
}

//######################################################################
//Returns a map of all known services, with empty keys.  This helps us manage un-deployment of expired services.
orderLib::Reconciliation::t_purchaseMap PurchaseManager::getEmptyServiceMap() {
    orderLib::Reconciliation::t_purchaseMap ret;

    //Add all services here!  The empty string represents an empty key, making sure the service
    //will be recognized and forced off.
    ret["utAdFree"] = "";

    return ret;
}

//######################################################################
void PurchaseManager::setServiceState(const std::string &serviceCode, orderLib::Key &keyObj, orderLib::Key &oldKeyObj) {
    //Save the new key.  We do this first even if it's a code we don't recognize, because we just sold it to them
    //and we should always save it.  An updated client would recognize it.
    tstring ts_serviceCode = to_tstring(serviceCode.c_str());
    if (keyObj) {
        //The new key is an active valid key.  Save it.
        RegKey regKey;
        tstring ts_licenseKey = to_tstring(keyObj.getRawKey().c_str());
        if (! regKey.Create(ROOTKEY, TPRODUCT_PURCHASEKEYS_KEYPATH)) throw std::runtime_error("failRegistry");
        regKey.WriteString(ts_serviceCode.c_str(), ts_licenseKey.c_str());
    }
    else {
        //The new key is not a valid purchase key.  Remove anything that's there.
        RegDelete(ROOTKEY, TPRODUCT_PURCHASEKEYS_KEYPATH, ts_serviceCode.c_str());
    }

    //Set the service by code.
    if (serviceCode == "utAdFree") return PurchaseManager::setServiceState_utAdFree(keyObj, oldKeyObj);

    //Nothing matches.
    std::string msg("unknownPurchaseCode_");
    msg += serviceCode;
    throw std::invalid_argument(msg);
}

//######################################################################
void PurchaseManager::setServiceState_utAdFree(orderLib::Key &keyObj, orderLib::Key &oldKeyObj) {
    //Simple: turn ads on or off.  We don't need to look at the oldKeyObj
    bool isOn = (keyObj ? false : true);
    ::PostMessage(g_wndoverview, WM_SET_ADS_SHOW, isOn, 0);
}

//######################################################################
tstring PurchaseManager::getCustomerId() throw() {
    tstring customerId;
    RegQueryString(ROOTKEY, TPRODUCT_PURCHASES_KEYPATH, TPRODUCT_CUSTOMERID_SUBKEY, customerId);
    return customerId;
}

//######################################################################
tstring PurchaseManager::getCustomerJson() {
    std::string customerId = to_string(PurchaseManager::getCustomerId()).c_str();
    return to_tstring(orderLib::Reconciliation::getCustomerJson(customerId));
//	return tstring(_T(""));
}

//######################################################################
void PurchaseManager::saveCustomerId(const tstring& customerId) {
    RegKey regKey;
    if (! regKey.Create(ROOTKEY, TPRODUCT_PURCHASES_KEYPATH)) {
        throw std::runtime_error("failRegistry");
    }

    regKey.WriteString(TPRODUCT_CUSTOMERID_SUBKEY, customerId.c_str());
}

//######################################################################
//Returns a map of svcCode -> keyString for all current purchases.
orderLib::Reconciliation::t_purchaseMap PurchaseManager::getAllSavedPurchases() {
    //Read all from the registry.
    orderLib::Reconciliation::t_purchaseMap ret;
    Vector<Pair<tstring, tstring> > values = RegEnumValues(HKEY_CURRENT_USER, TPRODUCT_PURCHASEKEYS_KEYPATH);
    for (int i = 0; i < values.size(); ++i) {
        std::string k = to_string(values[i].first).c_str();
        std::string v = to_string(values[i].second).c_str();
        ret[k] = v;
    }

    return ret;
}

//######################################################################
void PurchaseManager::reconcilePurchaseState_callback(orderLib::Reconciliation::t_customerInfoMap *cust, orderLib::Reconciliation::t_purchaseMap *purchases) throw() {
    //If purchases is NULL, that's an error.  Empty purchases is fine, but NULL indicates something failed.
    //We don't wan to pass it as if we succeeded, that would be interpreded as the customer having no purchases.
    if (! purchases) {
        //Oops.
		BenchThread::ping(t_iVal::USER_INTERACTION, "", "Reconciliation_queryServer_fail");
        PurchaseManager::reconcilePurchaseState_handleFailure();
        return;
    }

    //We really should have cust too if we have purchases... but we choose not to trip if we don't
    if (cust) {
        //Do we have a customer id?
        std::string newCustomerId = (*cust)["custId"];
        if (! newCustomerId.empty()) {
            //Yep.  Is it different?
            std::string oldCustomerId = to_string(PurchaseManager::getCustomerId()).c_str();
            if (newCustomerId != oldCustomerId) {
                //Yep.  It's not required, but if it's passed then we save it.  This will allow us to change the customer ID if we need to.
                PurchaseManager::saveCustomerId(to_tstring(newCustomerId));
            }
        }
    }

    //Do the reconciliation
    PurchaseManager::reconcilePurchaseState_continue(purchases);
}

//######################################################################
void PurchaseManager::reconcilePurchaseState() throw() {
    //Get the customer id as tstring.
    std::string customerId = to_string(PurchaseManager::getCustomerId()).c_str();
    if (customerId.empty()) {
        //Nothing there, nothing to reconcile
        return;
    }

    //Do the reconciliation.  This is done in a thread.
    try {
        orderLib::Reconciliation recon(customerId);
        recon.queryServer(&(PurchaseManager::reconcilePurchaseState_callback));
        return;
    }
    catch (std::exception &Ex) {
        //Couldn't start it.  Log and flow through.
        string jsonErr;
		jsonErr += BenchThread::AddJson("error", string(Ex.what()));
		BenchThread::ping(t_iVal::USER_INTERACTION, jsonErr, "Reconciliation_queryServer");
    }

    //Failed.  We should still reconcile but do it through this fail function.
    PurchaseManager::reconcilePurchaseState_handleFailure();
}

//######################################################################
void PurchaseManager::reconcilePurchaseState_handleFailure() throw() {

    /*
   ***
   Should we count the number of failures in a row here, and kill all purchases after a certain number?
   ***
   */
    
    //We can't get purchase state from the server.  Just continue with what we have saved.
    try {
        PurchaseManager::reconcilePurchaseState_continue(NULL);
    }
    catch (std::exception &Ex) {
        string jsonErr;
        jsonErr += BenchThread::AddJson("error", string(Ex.what()));
		BenchThread::ping(t_iVal::USER_INTERACTION, jsonErr, "PurchaseManager_reconcilePurchaseState_handleFailure");
    }
}

//######################################################################
void PurchaseManager::reconcilePurchaseState_continue(const orderLib::Reconciliation::t_purchaseMap *serverPurchases) throw() {
    //Get our map of all known services, with empty keys.
    //This lets us reliably un-deploy any previously enabled services.
    orderLib::Reconciliation::t_purchaseMap allServices = PurchaseManager::getEmptyServiceMap();

    //Get the list of all saved purchases.
    orderLib::Reconciliation::t_purchaseMap savedPurchases = PurchaseManager::getAllSavedPurchases();

    //Do we have purchases from the server?
    const orderLib::Reconciliation::t_purchaseMap *relevantPurchases;
    if (serverPurchases) relevantPurchases = serverPurchases;
    else relevantPurchases = &savedPurchases;

    //Overwrite the empty keys with the valid(?) purchased keys from the server.
    for (auto &kv : *relevantPurchases) allServices[kv.first] = kv.second;
    
    //Set state for all purchases.
    for (auto &kv : allServices) {
        //Get the saved key if appropriate.
        std::string savedKey;
        try { savedKey = savedPurchases[kv.first]; }
        catch (...) { /* nothing there */ }

        //Make the two key objects.
        orderLib::Key relevantKeyObj(kv.second);
        orderLib::Key savedKeyObj(savedKey);

        try {
            //Set the state of the service.
            PurchaseManager::setServiceState(kv.first, relevantKeyObj, savedKeyObj);
        }
        catch (std::exception &Ex) {
            //Oops... something went wrong.
            string json;
            json += BenchThread::AddJson("error", string(Ex.what()));
			BenchThread::ping(t_iVal::USER_INTERACTION, json, "cannotReconcilePurchase");

            //Nothing else we can do.
            continue;
        }
    }
}

//######################################################################
std::string PurchaseManager::getUrl_customerManagement() {
    std::string ret("https://");
    ret += orderLib::Reconciliation::getHost();
    ret += "/customer/clientConnect";
    return ret;
}


/*
  Local Variables:
  c-basic-offset: 4
  End:
*/
