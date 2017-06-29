package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Microchipdirect;

public class MicrochipdirectSteps {

	Microchipdirect microchipdirect= new Microchipdirect(WebDriverFactory.get());

	@When("I login microchipdirect with '(.*)' and '(.*)'")
	public void login(String username,String password){
		microchipdirect.goToLogin();
		microchipdirect.enterEmail(username);
		microchipdirect.enterPassword(password);
		microchipdirect.submit();
		
	}
	@When("I go to microchipdirect login page")
	public void pressLogin(){
		microchipdirect.goToLogin();
		Assert.assertTrue("Expected to be at login page", microchipdirect.checkAtLoginPage());
		
	}
	@Then("I should be logged in microchipdirect")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",microchipdirect.checkLogin());
	}
	
	@When("I logout microchipdirect")
	public void pressLogout(){
		microchipdirect.logout();
	}
}
