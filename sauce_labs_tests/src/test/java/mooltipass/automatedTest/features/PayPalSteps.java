package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.PayPal;

public class PayPalSteps {
	PayPal paypal= new PayPal(WebDriverFactory.get());

	@When("I login PayPal with '(.*)'")
	public void login(String username){
		paypal.goToLogin();
		paypal.enterEmail(username);
		String password =System.getenv().get("PAYPALPASS");
		paypal.enterPassword(password);
		paypal.submit();
		
	}
	@When("I go to PayPal login page")
	public void pressLogin(){
		paypal.goToLogin();
		Assert.assertTrue("Expected to be at login page", paypal.checkAtLoginPage());
		
	}
	@Then("I should be logged in PayPal")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",paypal.checkLogin());
	}
	
	@When("I logout PayPal")
	public void pressLogout(){
		paypal.logout();
	}
}
