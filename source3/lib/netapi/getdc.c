/*
 *  Unix SMB/CIFS implementation.
 *  NetApi GetDC Support
 *  Copyright (C) Guenther Deschner 2007
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "includes.h"

#include "../librpc/gen_ndr/ndr_netlogon_c.h"
#include "librpc/gen_ndr/libnetapi.h"
#include "lib/netapi/netapi.h"
#include "lib/netapi/netapi_private.h"
#include "lib/netapi/libnetapi.h"
#include "libsmb/dsgetdcname.h"

/********************************************************************
********************************************************************/

WERROR NetGetDCName_l(struct libnetapi_ctx *ctx,
		      struct NetGetDCName *r)
{
	LIBNETAPI_REDIRECT_TO_LOCALHOST(ctx, r, NetGetDCName);
}

/********************************************************************
********************************************************************/

WERROR NetGetDCName_r(struct libnetapi_ctx *ctx,
		      struct NetGetDCName *r)
{
	NTSTATUS status;
	WERROR werr;
	struct dcerpc_binding_handle *b;
	const char *dcname;
	void *buffer;

	werr = libnetapi_get_binding_handle(ctx, r->in.server_name,
					    &ndr_table_netlogon,
					    &b);
	if (!W_ERROR_IS_OK(werr)) {
		goto done;
	}

	status = dcerpc_netr_GetDcName(b, talloc_tos(),
				       r->in.server_name,
				       r->in.domain_name,
				       &dcname,
				       &werr);

	if (!NT_STATUS_IS_OK(status)) {
		werr = ntstatus_to_werror(status);
		goto done;
	}

	if (!W_ERROR_IS_OK(werr)) {
		goto done;
	}

	if (NetApiBufferAllocate(strlen_m_term(dcname), &buffer)) {
		werr = WERR_NOT_ENOUGH_MEMORY;
		goto done;
	}
	memcpy(buffer, dcname, strlen_m_term(dcname));
	*r->out.buffer = buffer;
 done:

	return werr;
}

/********************************************************************
********************************************************************/

WERROR NetGetAnyDCName_l(struct libnetapi_ctx *ctx,
			 struct NetGetAnyDCName *r)
{
	LIBNETAPI_REDIRECT_TO_LOCALHOST(ctx, r, NetGetAnyDCName);
}

/********************************************************************
********************************************************************/

WERROR NetGetAnyDCName_r(struct libnetapi_ctx *ctx,
			 struct NetGetAnyDCName *r)
{
	NTSTATUS status;
	WERROR werr;
	struct dcerpc_binding_handle *b;
	const char *dcname;
	void *buffer;

	werr = libnetapi_get_binding_handle(ctx, r->in.server_name,
					    &ndr_table_netlogon,
					    &b);
	if (!W_ERROR_IS_OK(werr)) {
		goto done;
	}

	status = dcerpc_netr_GetAnyDCName(b, talloc_tos(),
					  r->in.server_name,
					  r->in.domain_name,
					  &dcname,
					  &werr);
	if (!NT_STATUS_IS_OK(status)) {
		werr = ntstatus_to_werror(status);
		goto done;
	}

	if (!W_ERROR_IS_OK(werr)) {
		goto done;
	}

	if (NetApiBufferAllocate(strlen_m_term(dcname), &buffer)) {
		werr = WERR_NOT_ENOUGH_MEMORY;
		goto done;
	}
	memcpy(buffer, dcname, strlen_m_term(dcname));
	*r->out.buffer = buffer;

 done:

	return werr;

}

/********************************************************************
********************************************************************/

WERROR DsGetDcName_l(struct libnetapi_ctx *ctx,
		     struct DsGetDcName *r)
{
	NTSTATUS status;
	struct libnetapi_private_ctx *priv;

	priv = talloc_get_type_abort(ctx->private_data,
		struct libnetapi_private_ctx);

	status = dsgetdcname(ctx,
			     priv->msg_ctx,
			     r->in.domain_name,
			     r->in.domain_guid,
			     r->in.site_name,
			     r->in.flags,
			     (struct netr_DsRGetDCNameInfo **)r->out.dc_info);
	if (!NT_STATUS_IS_OK(status)) {
		libnetapi_set_error_string(ctx,
			"failed to find DC: %s",
			get_friendly_nt_error_msg(status));
	}

	return ntstatus_to_werror(status);
}

/********************************************************************
********************************************************************/

WERROR DsGetDcName_r(struct libnetapi_ctx *ctx,
		     struct DsGetDcName *r)
{
	WERROR werr;
	NTSTATUS status;
	struct dcerpc_binding_handle *b;

	werr = libnetapi_get_binding_handle(ctx, r->in.server_name,
					    &ndr_table_netlogon,
					    &b);
	if (!W_ERROR_IS_OK(werr)) {
		goto done;
	}

	status = dcerpc_netr_DsRGetDCNameEx(b,
					    ctx,
					    r->in.server_name,
					    r->in.domain_name,
					    r->in.domain_guid,
					    r->in.site_name,
					    r->in.flags,
					    (struct netr_DsRGetDCNameInfo **)r->out.dc_info,
					    &werr);
	if (NT_STATUS_IS_OK(status) && W_ERROR_IS_OK(werr)) {
		goto done;
	}

	status = dcerpc_netr_DsRGetDCName(b,
					  ctx,
					  r->in.server_name,
					  r->in.domain_name,
					  r->in.domain_guid,
					  NULL,
					  r->in.flags,
					  (struct netr_DsRGetDCNameInfo **)r->out.dc_info,
					  &werr);
	if (!NT_STATUS_IS_OK(status)) {
		werr = ntstatus_to_werror(status);
		goto done;
	}

 done:
	return werr;
}
