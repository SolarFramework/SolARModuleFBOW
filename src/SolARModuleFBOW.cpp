/**
 * @copyright Copyright (c) 2015 All Right Reserved, B-com http://www.b-com.com/
 *
 * This file is subject to the B<>Com License.
 * All other rights reserved.
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 */

#include <iostream>

#include "xpcf/module/ModuleFactory.h"
#include "SolARModuleFBOW_traits.h"

#include "SolARKeyframeRetrieverFBOW.h"

namespace xpcf=org::bcom::xpcf;


#ifdef WITHCUDA
    XPCF_DECLARE_MODULE("0aa58e12-b5ac-11ec-b909-0242ac120002", "SolARModuleFBOWCuda", "SolARModuleFBOWCuda")
#else
    XPCF_DECLARE_MODULE("b81f0b90-bdbc-11e8-a355-529269fb1459", "SolARModuleFBOW", "SolARModuleFBOW")
#endif // WITHCUDA

extern "C" XPCF_MODULEHOOKS_API xpcf::XPCFErrorCode XPCF_getComponent(const boost::uuids::uuid& componentUUID,SRef<xpcf::IComponentIntrospect>& interfaceRef)
{
    xpcf::XPCFErrorCode errCode = xpcf::XPCFErrorCode::_FAIL;
    errCode = xpcf::tryCreateComponent<SolAR::MODULES::FBOW::SolARKeyframeRetrieverFBOW>(componentUUID,interfaceRef);

    return errCode;
}

XPCF_BEGIN_COMPONENTS_DECLARATION
XPCF_ADD_COMPONENT(SolAR::MODULES::FBOW::SolARKeyframeRetrieverFBOW)
XPCF_END_COMPONENTS_DECLARATION
